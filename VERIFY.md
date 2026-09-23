# CryptoMachine Seed Verification Guide

This guide explains how to independently verify the deterministic seed-generation process and, once production releases are published, how to verify a firmware artifact.

> **Use test data only.**
>
> Never enter a real recovery phrase into an internet-connected website or verification service.

## What You Can Verify

CryptoMachine Seed is designed so several important claims can be checked independently:

- the dice input is interpreted in a defined order
- the dice string is hashed with SHA-256
- 12-word generation uses the first 128 bits of the SHA-256 digest
- 24-word generation uses the full 256-bit SHA-256 digest
- BIP39 checksum calculation is standard
- the final mnemonic matches an independent BIP39 implementation
- a downloaded production UF2 matches the SHA-256 published for that release
- the release can be tied to an exact Git tag and source commit

A successful verification does not prove that every physical device is trustworthy. It provides evidence that the published algorithm, test vectors, source release, and firmware artifact agree.

## 1. Verify the Published Test Vectors

Start with `TESTVECTORS.md`.

Do not start with a real wallet seed.

### 12-word test vector

Use the published 50-character dice string exactly as written.

The expected processing path is:

```text
ASCII dice string
    |
    v
SHA-256
    |
    v
first 128 bits
    |
    v
BIP39 checksum
    |
    v
12-word mnemonic
```

### 24-word test vector

Use the published 100-character dice string exactly as written.

The expected processing path is:

```text
ASCII dice string
    |
    v
SHA-256
    |
    v
full 256-bit digest
    |
    v
BIP39 checksum
    |
    v
24-word mnemonic
```

The exact expected hashes, entropy values, checksum bits, and mnemonics are published in `TESTVECTORS.md`.

## 2. Verify the SHA-256 Dice Conditioning Yourself

Python can reproduce the SHA-256 step without using CryptoMachine Seed.

For the 12-word vector:

```powershell
python -c "import hashlib; s='65515223131652132161133154444123616466443112153441'; print(hashlib.sha256(s.encode('ascii')).hexdigest())"
```

Expected result:

```text
6cb09af855050dcde6fe2adc3181c250982011e2cf17821cbed56a908ec527c3
```

For the 24-word vector:

```powershell
python -c "import hashlib; s='1234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234'; print(hashlib.sha256(s.encode('ascii')).hexdigest())"
```

Expected result:

```text
e56403e8522ddeae1b44a1e8148b1ba4d3b4c626ccf20980056eedcc7e0c0f35
```

For 12 words, the first 32 hexadecimal characters are the 128-bit BIP39 entropy.

For 24 words, the complete 64-character digest is the 256-bit BIP39 entropy.

## 3. Verify BIP39 With an Independent Implementation

Do not rely only on CryptoMachine Seed to verify CryptoMachine Seed.

One widely used independent reference is Ian Coleman's BIP39 tool:

https://github.com/iancoleman/bip39

The project provides a standalone `bip39-standalone.html` file that can be downloaded and opened locally in a browser.

The online project page is:

https://iancoleman.io/bip39/

### Recommended process

1. Download the standalone release from the official Ian Coleman GitHub repository.
2. Verify that you obtained it from the intended project.
3. Disconnect the verification computer from the network.
4. Open `bip39-standalone.html` locally.
5. Use only the public test entropy or public test mnemonic from `TESTVECTORS.md`.
6. Confirm that the independently produced mnemonic matches the CryptoMachine Seed vector.
7. Close the browser and discard any test session data when finished.

Do not enter a real CryptoMachine Seed recovery phrase into the online version of the tool.

For real seed handling, the safest verification path is one that does not expose the recovery phrase to a network-connected general-purpose computer.

## 4. Verify a CryptoMachine Seed Hardware Test

For a hardware verification run:

1. Flash the intended test firmware.
2. Confirm the firmware version or source commit being tested.
3. Start a 12-word or 24-word test ceremony.
4. Enter the exact dice sequence from `TESTVECTORS.md`.
5. Confirm the final mnemonic matches the published expected mnemonic.
6. Open Entropy Details and confirm the expected roll count and BIP39 entropy size.
7. Return to the mnemonic review.
8. Select **Finish & Destroy**.
9. Confirm the device returns to a fresh state.
10. Power-cycle the device and confirm it returns to Home without displaying prior session material.

Use only the published test vectors for this procedure.

## 5. Verify a Published Firmware File

Production releases will publish the SHA-256 hash of the exact UF2 artifact.

On Windows PowerShell:

```powershell
Get-FileHash .\cryptomachine_seed.uf2 -Algorithm SHA256
```

Compare the complete resulting hash with the SHA-256 published in the official CryptoMachine Seed release notes.

A single differing character means the files are not identical.

Do not flash an artifact whose hash does not match the expected release hash.

## 6. Tie a Release to Source

A production release will identify:

- the release version
- the Git tag
- the exact Git commit
- the production UF2 filename
- the UF2 SHA-256
- release verification results

After cloning the repository, a reviewer can inspect the release tag and commit with standard Git commands.

Example:

```powershell
git show v1.0.0 --no-patch
git rev-parse v1.0.0^{commit}
```

The actual production tag will be documented in the corresponding release.

## 7. Run the Project Tests

The repository includes host-side regression tests for the portable core.

A clean host build should be followed by:

```powershell
ctest --test-dir .\build-host --output-on-failure
```

The RP2350 firmware should also be built cleanly from the documented source and toolchain before release comparison.

Production releases will document the hardened build configuration and release-verification procedure separately.

## What Verification Does Not Prove

Independent test vectors and matching firmware hashes are important, but they are not magic.

They do not prove:

- that a physical device has not been modified
- that every compiler or dependency is trustworthy
- that every possible software bug has been found
- that the firmware is protected by hardware-enforced secure boot
- that a real recovery phrase was handled safely after generation

See `SECURITY.md` for the project's security assumptions and known limitations.

## Reporting a Mismatch

If a published vector, hash, or release result does not match:

1. Stop before using the device with real funds.
2. Record the exact test input.
3. Record the firmware version or Git commit.
4. Record the observed hash, entropy, checksum, or mnemonic.
5. Report the mismatch privately through an official CryptoMachine contact channel.

Never send a real recovery phrase, private key, or wallet secret in a report.

---

**CryptoMachine Seed**

**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/product/cryptomachine-seed/
