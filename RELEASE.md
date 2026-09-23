# CryptoMachine Seed Production Release Process

This document describes the intended production release process for CryptoMachine Seed.

It is written for maintainers building an official CryptoMachine Seed firmware release.

> Do not use development builds as production firmware.
>
> Production signing private keys must remain outside this repository.

## Release Goals

An official production release should provide:

- a clean source checkpoint
- a versioned Git tag
- hardened production build settings
- a signed RP2350 UF2
- automated release verification
- host regression test results
- an exact SHA-256 hash for the released UF2
- release notes that identify the source commit and firmware artifact

## 1. Start From a Clean Repository

From the repository root:

```powershell
git status --short
git pull
git rev-parse --short HEAD
```

`git status --short` should return no output before beginning the release process.

Do not build an official release from a working tree with uncommitted changes.

## 2. Run Host Regression Tests

Build the host test target:

```powershell
cmake --build .\build-host --clean-first
```

Run the complete test suite:

```powershell
ctest --test-dir .\build-host --output-on-failure
```

All tests must pass before creating a production release.

## 3. Prepare Production Signing Material

The production firmware signing private key must be stored outside this repository.

Do not:

- commit the private key
- copy the private key into the repository tree
- upload the private key to GitHub
- include the private key in release artifacts
- share the private key with customers or testers

Back up the production signing key using an offline process appropriate for high-value signing material.

The public verification material may be distributed as required by the release tooling.

## 4. Configure a Dedicated Production Build Directory

Use a dedicated release build directory rather than the ordinary development build directory.

The production CMake configuration must enable:

```text
CRYPTOMACHINE_PRODUCTION_RELEASE=ON
CRYPTOMACHINE_DEV_LOGGING=OFF
CRYPTOMACHINE_SIGN_FIRMWARE=ON
CRYPTOMACHINE_STACK_USAGE_ANALYSIS=ON
```

Example configuration:

```powershell
cmake -S . -B .\build-rp2350-release `
  -DCRYPTOMACHINE_PRODUCTION_RELEASE=ON `
  -DCRYPTOMACHINE_DEV_LOGGING=OFF `
  -DCRYPTOMACHINE_SIGN_FIRMWARE=ON `
  -DCRYPTOMACHINE_STACK_USAGE_ANALYSIS=ON
```

If the signing configuration requires an external key path or other release-specific argument, provide it according to the project CMake configuration without copying the private key into the repository.

## 5. Build the Production Firmware

Build cleanly:

```powershell
cmake --build .\build-rp2350-release --clean-first
```

Do not substitute a previously generated development UF2 for the production artifact.

## 6. Run Automated Release Verification

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify-rp2350-release.ps1
```

The verifier is expected to check the hardened release configuration and release artifact.

A production release must not proceed if the verifier reports a failure.

The release verification currently covers checks including:

- required release artifacts exist
- development logging is disabled
- firmware signing is enabled
- production release guard is enabled
- first-party stack-usage analysis is enabled
- UF2 targets RP2350
- UF2 uses the expected ARM Secure image type
- UF2 signature verification
- production main stack reservation
- RP2350 stack-guard runtime support
- absence of known development USB or diagnostic runtime symbols
- first-party static stack-frame limits
- absence of known development diagnostic strings

## 7. Hardware Acceptance Test

Before publishing a production release, flash at least one known-good hardware unit with the exact release UF2.

Use test seeds only.

The acceptance test should include:

- clean boot to Home
- touchscreen operation
- 12-word deterministic test vector
- 24-word deterministic test vector
- Entropy Details
- return from Entropy Details to mnemonic review
- Finish & Destroy
- inactivity warning and automatic destruction
- power cycle after a completed ceremony
- confirmation that prior session material is not displayed after reboot
- basic abuse testing of navigation, back, cancel, and repeated taps
- dice-pattern warning behavior where practical

The expected deterministic vectors are documented in `TESTVECTORS.md`.

## 8. Compute the Release UF2 SHA-256

On Windows PowerShell:

```powershell
Get-FileHash .\build-rp2350-release\cryptomachine_seed.uf2 -Algorithm SHA256
```

Record the complete hash exactly.

The published hash must correspond to the exact UF2 distributed in the GitHub release.

## 9. Create the Release Commit and Tag

The release source must be committed before tagging.

Confirm:

```powershell
git status --short
git rev-parse HEAD
```

For the first public production release, the intended version is:

```text
v1.0.0
```

Create an annotated tag only after the source, tests, release verification, hardware acceptance test, and artifact hash are finalized.

Example:

```powershell
git tag -a v1.0.0 -m "CryptoMachine Seed v1.0.0"
git push origin v1.0.0
```

Do not reuse or move a published production tag.

If a release must be replaced, create a new version.

## 10. Publish the GitHub Release

The GitHub release should identify:

- release version
- Git tag
- exact Git commit
- firmware filename
- firmware SHA-256
- release verification status
- host test status
- supported reference hardware
- important security notes
- link to `SECURITY.md`
- link to `TESTVECTORS.md`
- link to `VERIFY.md`

Attach only public release artifacts.

Never attach production signing private keys or private provisioning material.

## 11. Verify the Published Artifact

After publishing, download the UF2 from the GitHub release as a user would.

Compute its SHA-256 again:

```powershell
Get-FileHash .\cryptomachine_seed.uf2 -Algorithm SHA256
```

Confirm that the downloaded file matches the SHA-256 published in the release notes.

This checks that the public download matches the artifact that passed the release process.

## 12. Production Unit Flashing

Production units should be flashed only with the exact approved production release artifact.

For each unit, record at minimum:

- hardware unit identifier or serial
- firmware version
- firmware SHA-256
- date flashed
- display and touch test result
- seed ceremony test result
- Finish & Destroy test result
- cold boot test result

Do not modify the production firmware per unit unless a new release is created.

## Secure Boot and OTP

Firmware signing and hardware-enforced secure boot are separate controls.

A signed UF2 does not by itself prove that RP2350 OTP secure-boot enforcement is active.

Any OTP provisioning procedure must be documented, reviewed, tested on sacrificial hardware first, and treated as potentially irreversible.

Do not improvise OTP commands during normal production flashing.

## Release Checklist

Before publishing a production release, confirm all of the following:

- [ ] Working tree is clean
- [ ] Host regression tests pass
- [ ] Production signing key remains outside the repository
- [ ] Production build settings are enabled
- [ ] Production build completes cleanly
- [ ] Automated release verifier passes
- [ ] 12-word hardware vector passes
- [ ] 24-word hardware vector passes
- [ ] Entropy Details works
- [ ] Finish & Destroy works
- [ ] Inactivity destruction works
- [ ] Cold boot returns to Home
- [ ] Release UF2 SHA-256 recorded
- [ ] Exact source commit recorded
- [ ] Version tag created
- [ ] GitHub release created
- [ ] Published UF2 downloaded and hash rechecked

## Related Documentation

- `README.md`
- `SECURITY.md`
- `TESTVECTORS.md`
- `VERIFY.md`
- `TRADEMARK.md`

---

**CryptoMachine Seed**

**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/product/cryptomachine-seed/
