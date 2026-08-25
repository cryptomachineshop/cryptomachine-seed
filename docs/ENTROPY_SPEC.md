# CryptoMachine Seed Generator
## Entropy Specification

Status: V1 DEVELOPMENT SPECIFICATION

This document defines the current CryptoMachine dice-to-BIP39 behavior.

The implementation must remain deterministic, independently reproducible, and covered by automated known-answer tests.

## 1. Entropy source

CryptoMachine does not generate seed entropy internally.

The user supplies physical entropy using five six-sided dice (D6).

The five dice have fixed identities. Production units are intended to use five distinguishable dice.

The UI must not rely on color alone. Each die should also have a fixed position, number, letter, or symbol.

## 2. Dice order

Every shake is entered in the same fixed order:

D1
D2
D3
D4
D5

Example:

D1 = 6
D2 = 2
D3 = 4
D4 = 1
D5 = 5

The canonical representation is:

62415

No commas, spaces, timestamps, device identifiers, hidden values, or separators are added.

## 3. Multiple shakes

Each confirmed five-dice result is appended directly to the previous result.

Example:

Shake 1: 62415
Shake 2: 35126
Shake 3: 44321

Canonical dice string:

624153512644321

The exact entered sequence determines the result.

## 4. 12-word production mode

12-word mode requires:

5 dice
x 10 shakes
= 50 dice outcomes

The 50-character dice string is encoded as ASCII and hashed with SHA-256.

The first 16 bytes of the SHA-256 digest are used as the 128-bit BIP39 entropy.

BIP39 then adds a 4-bit checksum.

Result:

128 entropy bits
+ 4 checksum bits
= 132 bits
= 12 BIP39 words

A published external 50-outcome reference vector is included in the automated test suite.

## 5. 24-word production mode

24-word mode requires:

5 dice
x 20 shakes
= 100 dice outcomes

The 100-character dice string is encoded as ASCII and hashed with SHA-256.

The complete 32-byte SHA-256 digest is used as the 256-bit BIP39 entropy.

BIP39 then adds an 8-bit checksum.

Result:

256 entropy bits
+ 8 checksum bits
= 264 bits
= 24 BIP39 words

CryptoMachine production mode requires exactly 100 outcomes.

## 6. 99-outcome compatibility reference

The project retains a published 99-outcome SeedSigner-style test vector.

This exists for:

- independent verification
- compatibility testing
- regression testing

It is not the normal CryptoMachine production workflow.

Production 24-word mode uses 100 outcomes so the customer completes exactly 20 full five-dice shakes.

## 7. BIP39 word indexing

BIP39 uses an ordered English word list containing exactly 2048 words.

Internal word indexes are:

0 through 2047

If the touchscreen displays human-friendly word numbers:

0001 through 2048

that display conversion must remain separate from the internal BIP39 index and must be tested for off-by-one errors.

## 8. Dice sanity checks

CryptoMachine may perform simple sanity checks on the entered sequence.

Current checks include warnings for:

- one or more faces never appearing
- unusually long repeated runs
- grossly disproportionate face counts

These checks are warnings only.

They do not prove that physical dice are fair.
They do not measure the exact entropy of a sequence.
They do not certify that a seed is secure.

A warning should allow the user to restart the ceremony or deliberately continue.

## 9. Determinism

The same exact dice sequence must always generate the same mnemonic.

Mnemonic generation must never depend on:

- device RNG
- clock or timestamp
- serial number
- hardware identifier
- session counter
- hidden salt
- CryptoMachine secret
- network source

## 10. Sensitive data

The following are considered sensitive during a live seed ceremony:

- entered dice sequence
- SHA-256 result
- derived entropy
- BIP39 checksum working data
- mnemonic indexes
- mnemonic words
- temporary UI copies of those values

Production firmware must not intentionally write this information to:

- flash
- TF or microSD
- logs
- debug output
- configuration storage
- crash dumps

Sensitive working memory must be explicitly cleared after use.

Cleanup behavior must also be reviewed for:

- cancellation
- inactivity timeout
- reset
- power interruption
- abnormal restart
- next boot

## 11. Current production test vector

The current CryptoMachine 100-outcome reference input is:

1234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234

Expected 24-word mnemonic:

tornado cactus wheel picture target finish home neither trend picture shoulder endless deputy glide open oxygen another ability forum swear side alcohol devote random

This vector must remain in the automated test suite.

Any code change that causes this result to change must fail testing.

## 12. External reference vectors

The automated tests also retain known external vectors for:

- 50-outcome 12-word generation
- 99-outcome 24-word generation
- official BIP39 entropy/checksum behavior

These provide an independent check against CryptoMachine-specific production behavior.

## 13. Manual BIP39 tools

The same core BIP39 implementation also supports:

- 11 entered words -> valid possible 12th checksum words
- 23 entered words -> valid possible 24th checksum words
- validation of complete 12-word mnemonics
- validation of complete 24-word mnemonics

These functions do not generate entropy.

## 14. Security philosophy

The intended trust model is:

The customer creates the entropy.
CryptoMachine performs transparent deterministic math.

The firmware should be open source and independently reviewable.

Published builds should include test vectors and firmware hashes.

Development builds must not be represented as production-ready for protecting funds until the firmware and hardware behavior have been adequately reviewed and tested.
