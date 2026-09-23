# CryptoMachine Seed Build Guide

This guide documents the known-working development build paths for CryptoMachine Seed.

It covers:

- host-side core tests
- optional host sanitizer testing
- RP2350 development firmware
- the current Windows toolchain configuration

For hardened production releases, use `RELEASE.md`.

> **Project status:** Pre-release / hardware beta.
>
> Use test seeds only until a production release is published.

## Repository

Official repository:

https://github.com/cryptomachineshop/cryptomachine-seed

Clone:

```powershell
git clone https://github.com/cryptomachineshop/cryptomachine-seed.git
Set-Location .\cryptomachine-seed
```

Before building, confirm the checkout:

```powershell
git status --short
git rev-parse --short HEAD
```

## Known-Working Windows Toolchain

The current development environment uses:

- Windows
- CMake 3.20 or newer
- Ninja
- Clang for host builds and tests
- Arm GNU Toolchain `arm-none-eabi` 12.2.rel1 for RP2350 firmware
- Raspberry Pi Pico SDK from an external checkout
- C++20 for the portable core and RP2350 C++ code

The current working RP2350 environment uses:

```text
Generator: Ninja
Build type: Release
PICO_BOARD: cryptomachine_rp2350_touch_lcd_3_5
PICO_PLATFORM: rp2350-arm-s
PICO_SDK_PATH: C:/CryptoMachine/pico-sdk
Arm GNU Toolchain: 12.2.rel1
```

The current working host environment uses:

```text
Generator: Ninja
Build type: Debug
C++ compiler: clang++
```

### Known-Working Toolchain Snapshot

The current validated development environment is:

```text
Raspberry Pi Pico SDK: 2.3.0
Pico SDK commit: 98a542c1a62fb549ffb5d66a3e5892b06276b670

CMake: 4.4.2
Ninja: 1.13.2
Clang: 22.1.8
Arm GNU Toolchain: 12.2.MPACBTI-Rel1
arm-none-eabi-gcc: 12.2.1 20230214

picotool: 2.3.0
picotool commit: 6f6458d792b93685a11423b244a585eaa99eafcf
```

The current build-tree copy of picotool is generated under `build-rp2350/_deps/` and is not installed globally or required to be on `PATH`.

The current Windows picotool build reports:

```text
picotool v2.3.0 (Windows, Clang-22.1.8, Debug)
USB support: not compiled in
```

This snapshot documents the environment that is known to build the project successfully. The final production release should record its own exact toolchain and dependency versions again before the release artifact is published.

The local Pico SDK path shown above is only an example. Use the actual path to your Pico SDK checkout.

Before public V1, the exact Pico SDK revision used for the production release should be recorded in the release notes.

## 1. Host Build and Regression Tests

The root `CMakeLists.txt` builds the portable core and 14 host regression test executables.

### Configure

From the repository root:

```powershell
cmake -S . -B .\build-host -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_CXX_COMPILER=clang++
```

### Build

```powershell
cmake --build .\build-host
```

### Run all host tests

```powershell
ctest --test-dir .\build-host --output-on-failure
```

A successful current build runs 14 tests.

## 2. Optional Host Sanitizer Build

The host project supports AddressSanitizer and UndefinedBehaviorSanitizer through:

```text
CRYPTOMACHINE_HOST_SANITIZERS=ON
```

On Windows, the current project configuration requires Clang and x64 for sanitizer builds.

Use a separate build directory:

```powershell
cmake -S . -B .\build-host-sanitize -G Ninja `
  -DCMAKE_BUILD_TYPE=RelWithDebInfo `
  -DCMAKE_CXX_COMPILER=clang++ `
  -DCRYPTOMACHINE_HOST_SANITIZERS=ON
```

Build:

```powershell
cmake --build .\build-host-sanitize
```

Run:

```powershell
ctest --test-dir .\build-host-sanitize --output-on-failure
```

Sanitizer failures must be investigated before a production release.

## 3. RP2350 Development Firmware

The RP2350 firmware has its own CMake project under:

```text
firmware/rp2350/
```

The project requires `PICO_SDK_PATH` to point to a Raspberry Pi Pico SDK checkout.

### Example environment

Current development machine:

```text
PICO_SDK_PATH=C:/CryptoMachine/pico-sdk
```

Your path may be different.

### Configure

From the repository root:

```powershell
cmake -S .\firmware\rp2350 -B .\build-rp2350 -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DPICO_SDK_PATH="C:/CryptoMachine/pico-sdk"
```

The RP2350 CMake project selects the custom board:

```text
cryptomachine_rp2350_touch_lcd_3_5
```

The known-working platform is:

```text
rp2350-arm-s
```

The board definition is stored under:

```text
firmware/rp2350/boards/
```

### Build

```powershell
cmake --build .\build-rp2350
```

For a fully clean rebuild:

```powershell
cmake --build .\build-rp2350 --clean-first
```

The expected UF2 output is:

```text
build-rp2350/cryptomachine_seed.uf2
```

## 4. Development Firmware Defaults

The RP2350 project defaults to:

```text
CRYPTOMACHINE_DEV_LOGGING=OFF
CRYPTOMACHINE_SIGN_FIRMWARE=OFF
CRYPTOMACHINE_PRODUCTION_RELEASE=OFF
CRYPTOMACHINE_STACK_USAGE_ANALYSIS=OFF
```

These defaults are appropriate for ordinary local development and hardware testing.

They are not the complete production release configuration.

Use `RELEASE.md` for an official production build.

## 5. Development Logging

Development USB logging can be enabled explicitly when needed:

```powershell
cmake -S .\firmware\rp2350 -B .\build-rp2350-log -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DPICO_SDK_PATH="C:/CryptoMachine/pico-sdk" `
  -DCRYPTOMACHINE_DEV_LOGGING=ON
```

Do not enable development logging in a production release.

Sensitive dice input, entropy, mnemonic data, or other seed material must never be added to diagnostic logging.

## 6. Firmware Signing

Firmware signing is optional for normal development builds and mandatory for the intended official production release process.

The signing option is:

```text
CRYPTOMACHINE_SIGN_FIRMWARE=ON
```

When enabled, `CRYPTOMACHINE_SIGNING_KEY` must point to an external secp256k1 PEM private key.

The private signing key must remain outside this repository.

Do not use a production signing key for routine development testing.

See `RELEASE.md` for the hardened production process.

## 7. Main Stack and Stack Guards

The RP2350 firmware is configured with:

```text
PICO_STACK_SIZE=0x1000
PICO_USE_STACK_GUARDS=1
```

This reserves a 4096-byte main stack and enables RP2350 stack-guard support.

Production release verification checks these expectations.

## 8. Flashing a Development UF2

Put the Waveshare RP2350 board into BOOT mode.

On Windows, the board appears as a removable drive.

If the boot drive is `D:`, flash with:

```powershell
Copy-Item .\build-rp2350\cryptomachine_seed.uf2 D:\
```

The board should reboot automatically after the UF2 is copied.

Drive letters vary by system. Confirm the correct RP2350 boot volume before copying firmware.

## 9. Basic Hardware Check

After flashing a development build:

1. Confirm clean boot to Home.
2. Confirm touchscreen response.
3. Run a test seed ceremony.
4. Confirm mnemonic review renders correctly.
5. Confirm Entropy Details opens and returns correctly.
6. Confirm Finish & Destroy returns to a fresh state.
7. Power-cycle and confirm the prior test session is not displayed.

Use only test seeds during development.

Deterministic hardware test vectors are documented in `TESTVECTORS.md`.

## 10. Build Directory Hygiene

Generated build output should remain outside Git tracking.

The repository `.gitignore` excludes generated build directories and common firmware artifacts.

Examples:

```text
build-host/
build-rp2350/
build-*/
*.uf2
*.elf
*.bin
*.map
```

Do not commit generated firmware or local signing material directly to the source tree.

Official release artifacts should be distributed through the documented release process.

## 11. Reproducibility Notes

A source commit alone is not enough to fully reproduce a firmware binary if the toolchain or SDK changes.

For an official V1 release, record at minimum:

- exact CryptoMachine Seed Git commit
- exact Git tag
- Pico SDK revision
- Arm GNU Toolchain version
- CMake version
- Ninja version
- picotool version and source revision
- production CMake options
- firmware SHA-256

These values should accompany the official release.

## Related Documentation

- `README.md`
- `SECURITY.md`
- `TESTVECTORS.md`
- `VERIFY.md`
- `RELEASE.md`
- `TRADEMARK.md`

---

**CryptoMachine Seed**

**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/product/cryptomachine-seed/
