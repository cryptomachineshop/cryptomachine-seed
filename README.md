# CryptoMachine Seed

**Roll. Verify. Write. Destroy.**

CryptoMachine Seed is a dedicated offline BIP39 seed phrase generator that uses user-supplied physical dice entropy.

The device is designed to do one job well: guide a user through creating a 12-word or 24-word BIP39 recovery phrase from physical dice entropy, let the result be reviewed, and then clear the active ceremony.

> **Project status:** Pre-release / hardware beta.
> Use **test seeds only** until a production release is published.

## Why CryptoMachine Seed?

CryptoMachine Seed is built around a simple idea: the randomness starts with dice in your hands.

Instead of relying on an online service, phone app, browser, cloud-connected device, or manually working through a printed list of 2,048 BIP39 words, the device guides the ceremony locally on dedicated hardware.

The intended workflow is:

**Roll → Enter → Generate → Verify → Write Down → Destroy**

## Current V1 Design

CryptoMachine Seed currently supports:

- 12-word BIP39 recovery phrases
- 24-word BIP39 recovery phrases
- Physical D6 dice as the entropy source
- 50 dice rolls for a 12-word phrase
- 100 dice rolls for a 24-word phrase
- Guided touchscreen dice entry
- Automatic BIP39 checksum generation
- Full mnemonic review
- Entropy details
- Session destruction through **Finish & Destroy**
- Inactivity protection and automatic session clearing
- Manual BIP39 word-entry/checksum tooling

For deterministic seed generation, the entered dice digits are processed in order with SHA-256. The first 128 bits of the digest are used as BIP39 entropy for a 12-word phrase, or the full 256-bit digest for a 24-word phrase. The BIP39 checksum is then calculated automatically.

The user does not choose or guess a checksum word during the dice ceremony.

## Offline by Design

The current reference hardware has no Wi-Fi or Bluetooth radio.

CryptoMachine Seed does not require:

- Wi-Fi
- Bluetooth
- A cloud account
- Analytics
- Telemetry
- Synchronization
- A wallet connection
- A network connection

Seed generation takes place locally on the device.

## No Intentional Seed Persistence

CryptoMachine Seed is designed around a temporary session model.

Dice input, derived entropy, mnemonic state, and sensitive UI/session data are held only for the active ceremony and are not intentionally written to persistent storage.

When **Finish & Destroy** is selected, the application clears the active session and returns to a fresh state.

Production builds are designed with sensitive logging disabled.

Detailed security assumptions, destruction boundaries, limitations, and the threat model will be documented in `SECURITY.md`.

## What CryptoMachine Seed Does Not Do

CryptoMachine Seed is intentionally narrow in scope.

It does not:

- Connect to your wallet
- Sign Bitcoin transactions
- Store a recovery phrase for later retrieval
- Upload a seed
- Require an account
- Require a network service

It is a seed-generation appliance, not a wallet.

## Hardware

Current reference hardware:

- Waveshare RP2350-Touch-LCD-3.5
- RP2350B
- 3.5-inch 320×480 capacitive touchscreen
- USB-C power
- External USB-C data blocker recommended during normal use
- Five physical D6 dice

CryptoMachine Seed includes a portable core separated from the RP2350-specific hardware and UI layer.

## Repository Layout

The project is organized so deterministic seed-generation logic can be tested independently from the hardware target.

- `firmware/core/` : portable dice, SHA-256, BIP39, ceremony, state-machine, secure-zero, and application logic
- `firmware/rp2350/` : RP2350 hardware, display, touch, LVGL UI, and device entry point
- `tools/` : release and verification tooling
- host tests : deterministic and state-machine regression coverage

## Testing and Verification

The project includes host-side regression tests for the portable core and hardware-target build checks for RP2350 firmware.

Before the public V1 release, this repository will include:

- `TESTVECTORS.md` with deterministic 12-word and 24-word vectors
- `VERIFY.md` with independent verification instructions
- `SECURITY.md` with the threat model, assumptions, limitations, and reporting process
- reproducible production build and release-verification instructions

A public production release will identify:

- The exact Git commit
- The release tag
- The production UF2
- The UF2 SHA-256 hash
- Release verification results

## Independent Verification

Do not rely only on CryptoMachine Seed itself to prove CryptoMachine Seed is correct.

Public release documentation will include known test vectors and instructions for reproducing BIP39 results with independent software.

**Never enter a real recovery phrase into an internet-connected website for verification.** Use test data and offline tools.

## Production Release Security

Production signing keys and other private release material are **not stored in this repository**.

Production releases are intended to be built with hardened release settings, including production logging disabled, firmware signing enabled, production release guards enabled, and stack-usage analysis enabled.

The exact release procedure will be documented before V1 is made public.

## License and Branding

CryptoMachine Seed firmware is licensed under the **GNU General Public License version 3.0 (GPLv3)**.

See `LICENSE` for the complete software license.

**CryptoMachine** and **CryptoMachine Seed** branding are not licensed under the GPLv3. Forks and modified versions must not imply that they are official CryptoMachine products.

See `TRADEMARK.md` for the project branding policy.

## Safety

A BIP39 recovery phrase can control access to Bitcoin funds.

Treat every real recovery phrase as highly sensitive. Do not photograph it, email it, text it, upload it, or type it into an internet-connected device unless you fully understand the security implications.

During development and beta testing, use **test seeds only**.

---

**CryptoMachine Seed**
**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/
