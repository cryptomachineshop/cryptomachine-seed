# CryptoMachine Seed Quick Start

**Roll. Verify. Write. Destroy.**

CryptoMachine Seed is a dedicated offline BIP39 seed generator for creating 12-word or 24-word recovery phrases from physical six-sided dice.

## Before You Begin

A BIP39 recovery phrase can control access to Bitcoin funds.

Complete the ceremony privately and make sure you have a secure way to record and store the finished recovery phrase before you begin.

## 1. Power CryptoMachine Seed

Connect CryptoMachine Seed to USB power.

For normal use, place the included USB data blocker between the power cable and CryptoMachine Seed so the connection provides power without USB data.

## 2. Choose Your Seed Length

Select either:

- **12 words:** 50 dice results, typically 10 shakes of five dice
- **24 words:** 100 dice results, typically 20 shakes of five dice

## 3. Roll and Enter the Dice

Place all five dice in the shaker cup, shake, and roll them.

Each physical die color matches its corresponding position on the CryptoMachine Seed screen:

- **D1: Red**
- **D2: Blue**
- **D3: Green**
- **D4: Yellow**
- **D5: Purple**

Enter the result from each die in the matching on-screen position.

Repeat until all required dice results have been entered.

## 4. Verify

Review your entered dice results before completing the ceremony.

CryptoMachine Seed also provides an **Entropy Details** screen for users who want to inspect the deterministic generation process in greater detail.

For independent verification using test data, see:

- [`TESTVECTORS.md`](../TESTVECTORS.md)
- [`VERIFY.md`](../VERIFY.md)

## 5. Write Down Your Recovery Phrase

Carefully record the generated BIP39 recovery phrase on your preferred backup medium.

**Verify every word before continuing.**

## 6. Finish & Destroy

Once your recovery phrase has been securely recorded and verified, select **Finish & Destroy**.

CryptoMachine Seed clears the active ceremony and returns to a fresh state.

## Automatic Session Protection

If CryptoMachine Seed is left unattended during an active ceremony:

- **3 minutes:** display dims
- **9 minutes:** destruction warning and 60-second countdown
- **10 minutes:** active session is automatically destroyed

## Protect Your Recovery Phrase

Your recovery phrase can control access to your Bitcoin.

- Never photograph it.
- Never email or text it.
- Never upload it to cloud storage.
- Never enter it into an internet-connected website for verification.
- Verify every written word before destroying the active session.
- Store your completed backup somewhere secure.

## Offline by Design

CryptoMachine Seed does not require:

- Wi-Fi
- Bluetooth
- A cloud account
- Analytics
- Telemetry
- A wallet connection
- A network connection

CryptoMachine Seed is a seed-generation appliance. It is not a hardware wallet and does not sign Bitcoin transactions.

## Production Firmware

Current production firmware:

`v1.0.0`

UF2 SHA-256:

`CF8B726FC5CE1ED0482E131D595303D23A3C916681EFB837A6C84BD479658F4F`

See the GitHub Releases section for the signed production firmware and `SHA256SUMS.txt`.

## More Information

Product page:

https://cryptomachine.shop/product/cryptomachine-seed/

Online Quick Start Guide:

https://cryptomachine.shop/cryptomachine-seed-quick-start/

---

**CryptoMachine Seed**  
**Roll. Verify. Write. Destroy.**
