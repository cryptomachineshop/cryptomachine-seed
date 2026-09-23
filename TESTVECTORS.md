# CryptoMachine Seed Test Vectors

These deterministic vectors are provided so the CryptoMachine Seed dice to BIP39 process can be reproduced independently.

> **Use test data only.**
>
> The mnemonics in this document are public test vectors and must never be used to protect Bitcoin or any other funds.

## Generation Method

CryptoMachine Seed uses physical D6 results represented as ASCII digits `1` through `6`.

For the vectors below:

1. Record the dice results in the exact order shown.
2. Concatenate the digits with no spaces, commas, separators, or newline characters.
3. Encode that digit string as ASCII bytes.
4. Compute SHA-256 over those ASCII bytes.
5. For a 12-word phrase, use the first 128 bits of the SHA-256 digest as BIP39 entropy.
6. For a 24-word phrase, use the full 256-bit SHA-256 digest as BIP39 entropy.
7. Compute the standard BIP39 checksum from that entropy.
8. Convert the entropy plus checksum into BIP39 English word indexes.

The BIP39 checksum is generated automatically. The user does not select a checksum word.

## 12-Word Vector

### Dice input

50 physical dice results:

```text
65515223131652132161133154444123616466443112153441
```

The same input grouped into five dice per shake for readability:

```text
65515
22313
16521
32161
13315
44441
23616
46644
31121
53441
```

When hashing, use the single 50-character digit string with no separators.

### SHA-256 of ASCII dice string

```text
6cb09af855050dcde6fe2adc3181c250982011e2cf17821cbed56a908ec527c3
```

### BIP39 entropy

The first 128 bits of the digest are used:

```text
6cb09af855050dcde6fe2adc3181c250
```

### BIP39 checksum

4 checksum bits:

```text
0111
```

### Expected mnemonic

```text
hole luggage safe present express tragic orbit shed switch metal identify path
```

## 24-Word Vector

### Dice input

100 physical dice results:

```text
1234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234561234
```

For readability, the same sequence can be viewed as two 50-character lines:

```text
12345612345612345612345612345612345612345612345612
34561234561234561234561234561234561234561234561234
```

When hashing, concatenate the two lines exactly. Do not include a newline or any other separator.

### SHA-256 of ASCII dice string

```text
e56403e8522ddeae1b44a1e8148b1ba4d3b4c626ccf20980056eedcc7e0c0f35
```

### BIP39 entropy

The full 256-bit digest is used:

```text
e56403e8522ddeae1b44a1e8148b1ba4d3b4c626ccf20980056eedcc7e0c0f35
```

### BIP39 checksum

8 checksum bits:

```text
10001101
```

### Expected mnemonic

```text
tornado cactus wheel picture target finish home neither trend picture shoulder endless deputy glide open oxygen another ability forum swear side alcohol devote random
```

## What These Vectors Verify

A matching result provides evidence that an implementation agrees on:

- dice ordering
- ASCII encoding of the dice string
- SHA-256 conditioning
- 128-bit or 256-bit entropy selection
- BIP39 checksum calculation
- BIP39 English wordlist indexing
- final mnemonic generation

These vectors do not by themselves prove that a particular physical device is trustworthy or that the firmware running on that device matches a published source release.

## Independent Verification

The vectors should be checked with an implementation independent of CryptoMachine Seed.

For BIP39 verification, test data may also be compared using the offline standalone version of Ian Coleman's BIP39 tool.

Do not paste a real recovery phrase into an internet-connected website.

The public V1 release will include additional verification guidance in `VERIFY.md`.

## Reporting a Mismatch

If an independent implementation produces a different result, record:

- the exact dice string
- whether the test is 12-word or 24-word
- the SHA-256 digest
- the selected BIP39 entropy
- the calculated checksum bits
- the resulting mnemonic
- the CryptoMachine Seed firmware version or Git commit

Never include a real recovery phrase or private wallet material in a bug report.

---

**CryptoMachine Seed**

**Roll. Verify. Write. Destroy.**

https://cryptomachine.shop/product/cryptomachine-seed/
