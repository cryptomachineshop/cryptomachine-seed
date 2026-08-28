param(
    [Parameter(Mandatory = $false)]
    [string]$BuildDir = ".\build-rp2350-release"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Fail([string]$Message) {
    Write-Host "FAIL: $Message" -ForegroundColor Red
    exit 1
}

function Pass([string]$Message) {
    Write-Host "PASS: $Message"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildPath = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $repoRoot $BuildDir
}

if (-not (Test-Path $buildPath -PathType Container)) {
    Fail "Build directory does not exist: $buildPath"
}

$cachePath = Join-Path $buildPath "CMakeCache.txt"
$elfPath = Join-Path $buildPath "cryptomachine_seed.elf"
$uf2Path = Join-Path $buildPath "cryptomachine_seed.uf2"

foreach ($required in @($cachePath, $elfPath, $uf2Path)) {
    if (-not (Test-Path $required -PathType Leaf)) {
        Fail "Required release artifact is missing: $required"
    }
}
Pass "Required release artifacts exist"

$cache = Get-Content $cachePath -Raw

$requiredCacheValues = @{
    "CRYPTOMACHINE_PRODUCTION_RELEASE:BOOL=ON" = "production release guard is enabled"
    "CRYPTOMACHINE_DEV_LOGGING:BOOL=OFF"       = "development logging is disabled"
    "CRYPTOMACHINE_SIGN_FIRMWARE:BOOL=ON"      = "firmware signing is enabled"
    "CRYPTOMACHINE_STACK_USAGE_ANALYSIS:BOOL=ON" = "first-party stack usage analysis is enabled"
}

foreach ($entry in $requiredCacheValues.GetEnumerator()) {
    if ($cache -notmatch [regex]::Escape($entry.Key)) {
        Fail $entry.Value
    }
    Pass $entry.Value
}

$picotool = Get-ChildItem $buildPath -Recurse -Filter "picotool.exe" -File |
    Select-Object -First 1

if (-not $picotool) {
    Fail "picotool.exe was not found under the release build directory"
}

$picotoolInfo = (& $picotool.FullName info -a $uf2Path 2>&1 | Out-String)
if ($LASTEXITCODE -ne 0) {
    Fail "picotool could not inspect the UF2"
}

if ($picotoolInfo -notmatch '(?m)^\s*target chip:\s+RP2350\s*$') {
    Fail "UF2 target chip is not reported as RP2350"
}
Pass "UF2 targets RP2350"

if ($picotoolInfo -notmatch '(?m)^\s*image type:\s+ARM Secure\s*$') {
    Fail "UF2 is not reported as ARM Secure"
}
Pass "UF2 is ARM Secure"

if ($picotoolInfo -notmatch '(?m)^\s*signature:\s+verified\s*$') {
    Fail "UF2 signature did not verify"
}
Pass "UF2 signature is verified"

$nm = Get-Command "arm-none-eabi-nm.exe" -ErrorAction SilentlyContinue
if (-not $nm) {
    $compilerLine = ($cache -split "`r?`n" |
        Where-Object { $_ -like "CMAKE_C_COMPILER:FILEPATH=*" } |
        Select-Object -First 1)

    if ($compilerLine) {
        $compilerPath = $compilerLine.Substring($compilerLine.IndexOf("=") + 1)
        $candidate = Join-Path (Split-Path $compilerPath -Parent) "arm-none-eabi-nm.exe"
        if (Test-Path $candidate -PathType Leaf) {
            $nm = Get-Item $candidate
        }
    }
}

if (-not $nm) {
    Fail "arm-none-eabi-nm.exe was not found"
}

$nmPath = if ($nm.PSObject.Properties.Name -contains "Source") {
    $nm.Source
} else {
    $nm.FullName
}

$symbols = (& $nmPath -C $elfPath 2>&1 | Out-String)
if ($LASTEXITCODE -ne 0) {
    Fail "arm-none-eabi-nm could not inspect the ELF"
}

$stackBottomMatch = [regex]::Match(
    $symbols,
    '(?m)^([0-9A-Fa-f]+)\s+[A-Za-z]\s+__StackBottom\s*$'
)
$stackTopMatch = [regex]::Match(
    $symbols,
    '(?m)^([0-9A-Fa-f]+)\s+[A-Za-z]\s+__StackTop\s*$'
)

if (-not $stackBottomMatch.Success -or -not $stackTopMatch.Success) {
    Fail "Could not resolve __StackBottom/__StackTop from production ELF"
}

$stackBottom = [Convert]::ToInt64($stackBottomMatch.Groups[1].Value, 16)
$stackTop = [Convert]::ToInt64($stackTopMatch.Groups[1].Value, 16)
$mainStackBytes = $stackTop - $stackBottom

if ($mainStackBytes -ne 4096) {
    Fail "Production main stack is $mainStackBytes bytes; expected exactly 4096"
}
Pass "Production main stack reservation is 4096 bytes"

if ($symbols -notmatch '(?m)\bruntime_init_per_core_install_stack_guard\b') {
    Fail "RP2350 stack-guard runtime support is not present in production ELF"
}
Pass "RP2350 stack-guard runtime support is present"

$forbiddenSymbolPattern = 'stdio_init_all|stdio_usb|tud_cdc|tud_|usbd_|tinyusb|reset_usb_boot|rom_reset_usb_boot'
if ($symbols -match $forbiddenSymbolPattern) {
    $matches = ($symbols -split "`r?`n" |
        Where-Object { $_ -match $forbiddenSymbolPattern } |
        Select-Object -First 12) -join "`n"
    Fail "Unexpected USB/dev symbols found in production ELF:`n$matches"
}
Pass "Production ELF contains no known USB/dev runtime symbols"

$maxAllowedFrameBytes = 1024
$stackUsageFiles = Get-ChildItem $buildPath -Recurse -Filter "*.su" -File

if (-not $stackUsageFiles) {
    Fail "No compiler .su stack-usage reports were found in the production build"
}

$firstPartyFrames = foreach ($file in $stackUsageFiles) {
    foreach ($line in Get-Content $file.FullName) {
        if ($line -match '^(.*)\t(\d+)\t(.*)$') {
            $functionName = $matches[1]
            $frameBytes = [int]$matches[2]
            $frameType = $matches[3]

            if (
                $functionName -match 'cryptomachine-seed[\\/]+firmware[\\/]' -and
                $functionName -notmatch '[\\/]third_party[\\/]'
            ) {
                [PSCustomObject]@{
                    Function = $functionName
                    Bytes = $frameBytes
                    Type = $frameType
                }
            }
        }
    }
}

if (-not $firstPartyFrames) {
    Fail "No first-party stack-usage records were found"
}

$largestFrame = $firstPartyFrames |
    Sort-Object Bytes -Descending |
    Select-Object -First 1

if ($largestFrame.Bytes -gt $maxAllowedFrameBytes) {
    Fail (
        "First-party static stack frame exceeds budget: " +
        "$($largestFrame.Bytes) bytes > $maxAllowedFrameBytes bytes`n" +
        "$($largestFrame.Function)"
    )
}

Pass (
    "Largest first-party static stack frame is " +
    "$($largestFrame.Bytes) bytes (limit $maxAllowedFrameBytes)"
)

$strings = Get-Command "arm-none-eabi-strings.exe" -ErrorAction SilentlyContinue
if (-not $strings) {
    $candidate = Join-Path (Split-Path $nmPath -Parent) "arm-none-eabi-strings.exe"
    if (Test-Path $candidate -PathType Leaf) {
        $strings = Get-Item $candidate
    }
}

if (-not $strings) {
    Fail "arm-none-eabi-strings.exe was not found"
}

$stringsPath = if ($strings.PSObject.Properties.Name -contains "Source") {
    $strings.Source
} else {
    $strings.FullName
}

$binaryStrings = (& $stringsPath $elfPath 2>&1 | Out-String)
if ($LASTEXITCODE -ne 0) {
    Fail "arm-none-eabi-strings could not inspect the ELF"
}

$forbiddenStrings = @(
    "CryptoMachine Seed RP2350 bring-up",
    "SeedAppController ready",
    "RUNTIME FAULT",
    "STARTUP HALTED"
)

foreach ($text in $forbiddenStrings) {
    if ($binaryStrings.Contains($text)) {
        Fail "Development diagnostic string found in production ELF: $text"
    }
}
Pass "Known development diagnostic strings are absent"

Write-Host ""
Write-Host "CryptoMachine Seed RP2350 release verification PASSED."
exit 0
