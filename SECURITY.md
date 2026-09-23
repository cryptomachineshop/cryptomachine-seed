# Security Policy

CryptoMachine Seed is a dedicated offline BIP39 seed phrase generator that uses user-supplied physical dice entropy.

This document describes the current security model, assumptions, limitations, and reporting expectations for the project.

> **Project status:** Pre-release / hardware beta.
>
> Use **test seeds only** until a production release is published.

## Security Goals

CryptoMachine Seed is designed to:

- Generate BIP39 recovery phrases from user-supplied physical dice entropy
- Operate without Wi-Fi, Bluetooth, cloud services, accounts, telemetry, or network dependencies
- Avoid intentional persistence of dice inputs, derived entropy, mnemonic data, and other session secrets
- Clear sensitive session state at explicit destruction, timeout, cancellation, and fault boundaries
- Disable sensitive logging in production builds
- Make the deterministic dice-to-BIP39 process independently testable

CryptoMachine Seed is not a wallet and does not sign Bitcoin transactions.

## Core Principles

- The user supplies entropy using physical D6 dice.
- The device does not rely on an internal RNG for seed generation.
- Dice input is processed deterministically.
- BIP39 generation follows the public BIP39 specification.
- Sensitive seed material is intended to exist only during the active session.
- Seed words, entropy, and dice history are not intentionally written to persistent storage.
- Sensitive working memory is explicitly cleared at security boundaries.
- Production firmware must not log seed words, entropy, or dice results.
- The reference hardware has no Wi-Fi or Bluetooth radio.
- A physical USB data blocker is recommended for normal customer use.

## Threat Model

The primary goal is to reduce exposure of seed material during generation.

The design assumes:

- The user controls the physical device during the ceremony.
- The user supplies the physical dice entropy.
- The device is running the intended firmware.
- The hardware has not been maliciously modified before or during use.
- The user records and protects the resulting recovery phrase securely.

The project is primarily designed to reduce accidental persistence, unnecessary network exposure, cloud dependence, telemetry, and ordinary software paths that could retain sensitive session data.

## Offline Design

The current reference hardware does not include Wi-Fi or Bluetooth radio hardware.

CryptoMachine Seed does not require:

- Wi-Fi
- Bluetooth
- A cloud account
- Analytics
- Telemetry
- Synchronization
- A wallet connection
- A network service

Seed generation takes place locally on the device.

## Sensitive Session Data

During an active ceremony, sensitive state can include:

- Dice input
- Derived entropy
- Temporary SHA-256 material
- Generated mnemonic state
- Sensitive UI text and state
- Temporary review and warning state

The firmware is designed so this information is not intentionally written to persistent storage.

## Finish & Destroy

The **Finish & Destroy** flow clears the active seed ceremony and returns the application to a fresh state.

Sensitive buffers are explicitly cleared using the project's secure-zero implementation at security boundaries.

Additional clearing paths are used for inactivity timeout, cancellation, fault handling, and other transitions where sensitive state must not remain active.

The display is also cleared at destruction and fault boundaries so mnemonic text is not intentionally left visible in display memory.

## Inactivity Protection

Sensitive sessions include inactivity handling.

The current policy includes:

- display dimming after inactivity
- a visible timeout warning before destruction
- automatic session destruction after the configured timeout
- suppression of the wake touch so a held finger does not accidentally activate a control after wake

The Home screen is not treated as an active sensitive session.

## Runtime Fault Handling

Detected runtime or UI faults are handled fail-closed.

The intended fault path includes:

- turning off the backlight
- clearing active session data
- clearing fixed UI/session buffers
- clearing the LVGL draw buffer
- clearing LCD display memory
- emitting only a generic non-secret diagnostic message when development logging is available
- halting until power cycle

## Logging

Production firmware is designed with development logging disabled.

Sensitive dice input, entropy, mnemonic data, and other seed material must never be added to production logs.

## Dice Pattern Warnings

Dice-pattern sanity checks are intended as warnings only.

A warning must not modify:

- the entered dice sequence
- derived entropy
- the BIP39 checksum
- the generated mnemonic

Suspicious-looking randomness is not proof that the dice are invalid. The user remains responsible for the physical entropy source.

## Firmware Releases

Production releases are intended to use hardened release settings including:

- development logging disabled
- firmware signing enabled
- production release guard enabled
- first-party stack-usage analysis enabled
- RP2350 stack-guard runtime support
- automated release verification

Public releases will identify the exact Git commit, release tag, firmware artifact, and SHA-256 hash.

Production signing private keys are not stored in this repository.

## Signed Firmware vs. Secure Boot

A signed firmware image is not the same thing as hardware-enforced secure boot.

Unless explicitly documented for a specific production unit, do not assume RP2350 OTP secure-boot enforcement has been provisioned.

Any future OTP or secure-boot provisioning procedure must be reviewed separately because OTP changes can be irreversible.

## Independent Verification

Users and reviewers should not rely only on CryptoMachine Seed itself to prove the implementation is correct.

Before public V1 release, the repository is intended to include:

- deterministic 12-word and 24-word test vectors
- independent BIP39 verification instructions
- reproducible build and release-verification guidance
- firmware hashes tied to exact source commits

Never enter a real recovery phrase into an internet-connected website for verification. Use test data and offline tools.

## Known Limitations

CryptoMachine Seed does **not** claim protection against every possible attack.

In particular:

- It is not designed as a tamper-resistant hardware security module.
- It does not guarantee resistance to invasive physical attacks.
- It does not guarantee erasure of every transient CPU register or microarchitectural state.
- It cannot protect a seed that is photographed, copied, exposed, or stored insecurely after generation.
- It cannot protect against malicious hardware modification performed before the user receives the device.
- A signed firmware artifact alone does not prove hardware-enforced secure boot.
- Security depends on authentic firmware, trusted hardware, and safe seed-handling practices.

## Security Testing

The project includes host-side regression tests and RP2350 release checks.

Before public V1 release, this repository is intended to include:

- `TESTVECTORS.md`
- `VERIFY.md`
- production build instructions
- release verification instructions
- exact production release hashes

## Reporting a Security Issue

Please do not publish a suspected vulnerability before CryptoMachine has had a reasonable opportunity to investigate it.

During the pre-release phase, report security concerns privately through an official CryptoMachine contact channel.

When reporting an issue, include:

- firmware version or Git commit
- hardware version
- exact steps to reproduce
- whether test or real seed material was involved
- photos, logs, or other non-secret evidence that helps reproduce the issue

**Never send a real recovery phrase, private key, or wallet secret as part of a bug report.**

A dedicated public vulnerability-reporting channel will be documented before the repository is made public.

## Scope

This policy covers the CryptoMachine Seed firmware and the documented reference hardware/software configuration.

Third-party wallets, backup products, verification tools, operating systems, browsers, and other external systems are outside the scope of this repository.

---

**CryptoMachine Seed**

**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/product/cryptomachine-seed/
