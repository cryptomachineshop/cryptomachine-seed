# CryptoMachine Seed Generator

Offline BIP39 mnemonic generation and verification using user-supplied physical entropy.

## Project goals

- Generate 12-word and 24-word BIP39 mnemonics
- Use physical dice as the entropy source
- No Wi-Fi or Bluetooth hardware
- No intentional seed storage
- Open-source and independently verifiable
- Deterministic dice-to-BIP39 conversion
- Touchscreen interface for the Waveshare RP2350-Touch-LCD-3.5
- Support manual BIP39 word entry and checksum calculation

## Hardware

Target hardware:

- Waveshare RP2350-Touch-LCD-3.5
- RP2350B
- 3.5-inch 320x480 capacitive touchscreen
- USB-C power
- External USB-C data blocker during normal use
- Five physical D6 dice

## Status

Early development.

The cryptographic specification, test vectors, firmware, UI, and hardware behavior must be independently reviewed and tested before this project is considered suitable for protecting real Bitcoin.
