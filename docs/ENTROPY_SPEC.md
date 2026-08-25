# Entropy Specification

Status: DRAFT v0.1

This document defines how CryptoMachine Seed Generator will convert physical dice input into BIP39 entropy.

Nothing in this document should be considered final until the algorithm has been independently reviewed and verified against known test vectors.

## 1. Entropy source

CryptoMachine does not generate seed entropy internally.

The user supplies entropy using five physical six-sided dice (D6).

Each die has a permanent identity based on its color or other visible marking.

The touchscreen always requests the dice in the same fixed order:

D1
D2
D3
D4
D5

Color must not be the only identifier. The production UI should also identify each die by position, number, letter, or symbol.

## 2. One shake

The user shakes all five dice physically.

The five results are entered in fixed D1 through D5 order.

Example:

D1 = 6
D2 = 2
D3 = 4
D4 = 1
D5 = 5

The canonical representation of that shake is:

62415

No commas, spaces, separators, or hidden values are added.

## 3. Multiple shakes

Each confirmed shake is appended to the previous results in exact entered order.

Example:

Shake 1: 62415
Shake 2: 35126
Shake 3: 44321

Canonical dice string:

624153512644321

The exact canonical dice string must be reproducible by an independent implementation.

## 4. 12-word mode

BIP39 requires 128 bits of initial entropy for a 12-word mnemonic.

The final required number of physical dice outcomes is NOT YET LOCKED.

Candidates to evaluate:

- 50 outcomes for compatibility with established dice workflows
- 75 outcomes for additional margin against ordinary physical dice bias

The complete canonical dice string will be processed by SHA-256.

The method used to derive exactly 128 bits from the SHA-256 result must be explicitly documented, tested, and independently reviewed before release.

## 5. 24-word mode

BIP39 requires 256 bits of initial entropy for a 24-word mnemonic.

The final required number of physical dice outcomes is NOT YET LOCKED.

Candidates to evaluate:

- 99 outcomes for compatibility with established SeedSigner-style workflows
- 100 outcomes for exactly 20 five-dice shakes
- 150 outcomes for additional margin against ordinary physical dice bias

The complete canonical dice string will be processed by SHA-256.

The 256-bit SHA-256 result can then serve as the BIP39 entropy if this method is confirmed during review and compatibility testing.

## 6. BIP39 conversion

The resulting entropy is converted according to BIP39.

For 12 words:

128 entropy bits
+ 4 checksum bits
= 132 bits
= 12 groups of 11 bits

For 24 words:

256 entropy bits
+ 8 checksum bits
= 264 bits
= 24 groups of 11 bits

Each 11-bit value indexes the official 2048-word BIP39 English word list.

Internal BIP39 indexes are 0 through 2047.

If the user interface displays word numbers as 0001 through 2048, that display conversion must be separately tested to prevent an off-by-one error.

## 7. Security requirements

- Dice input must never be intentionally written to persistent storage.
- Derived entropy must never be intentionally written to persistent storage.
- Mnemonic words must never be intentionally written to persistent storage.
- Production logs must never contain dice input, entropy, hashes, word indexes, or mnemonic words.
- Sensitive RAM buffers must be explicitly wiped at the end of the session.
- Sensitive RAM must also be handled correctly on cancel, reset, timeout, and abnormal restart.
- The compiler must not be allowed to optimize away sensitive-memory wiping.

## 8. Determinism

The same valid dice sequence must always generate exactly the same result.

No:

- device RNG
- timestamp
- serial number
- hardware identifier
- counter
- hidden salt
- CryptoMachine-specific secret

may influence mnemonic generation.

## 9. Verification

Before release, the implementation must include automated known-answer tests.

Testing should include:

- published BIP39 test vectors
- independent dice-to-mnemonic test vectors
- SeedSigner-compatible vectors where applicable
- 12-word mode
- 24-word mode
- repeated dice values
- boundary cases
- undo and re-entry behavior
- BIP39 word indexing
- checksum generation

Any firmware change that alters a known expected mnemonic must cause the automated test suite to fail.

## 10. Open questions

Before this specification becomes v1.0, resolve:

1. Final number of dice outcomes for 12-word mode.
2. Final number of dice outcomes for 24-word mode.
3. Exact 128-bit derivation rule for 12-word mode.
4. Whether SeedSigner compatibility should be an explicit mode.
5. Whether entropy sanity checks should warn about obviously suspicious dice sequences.
6. How much additional entropy margin is appropriate for inexpensive consumer dice.
7. Exact RAM zeroization implementation on RP2350.
