# Security

CryptoMachine Seed Generator is intended to minimize trust in the device itself.

## Core principles

- The user supplies entropy using physical dice.
- The device does not rely on an internal RNG for seed generation.
- Dice input is processed deterministically.
- BIP39 generation follows the public BIP39 specification.
- Sensitive seed material should exist only during the active session.
- Seed words, entropy, and dice history must not be intentionally written to persistent storage.
- Sensitive working memory must be cleared after use.
- Production firmware must not log seed words, entropy, or dice results.
- The target hardware has no Wi-Fi or Bluetooth radio.
- A physical USB data blocker is intended for normal customer use.

## Verification

The project will publish known input/output test vectors so independent implementations can verify dice-to-mnemonic results.

## Development status

This project is currently experimental and has not yet undergone a completed independent security review.

Do not use development builds to protect funds.
