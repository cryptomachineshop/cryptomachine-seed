# CryptoMachine Seed Generator
## UI / Workflow Specification

Status: V1 DEVELOPMENT SPECIFICATION

This document defines the intended touchscreen behavior for CryptoMachine Seed Generator V1.

The visual design may change during hardware testing. The workflow and safety behavior should remain deliberate and predictable.

---

## 1. Boot

On power-up:

- initialize display and touch
- initialize a clean session
- clear sensitive working buffers
- do not restore any previous seed ceremony
- do not display previous dice input or mnemonic data

Initial screen:

CRYPTOMACHINE
SEED TOOL

Offline BIP39 tools

[ CONTINUE ]

---

## 2. Home screen

Primary options:

[ CREATE FROM DICE ]

[ CALCULATE FINAL WORD ]

[ VALIDATE BIP39 PHRASE ]

Secondary option:

[ ABOUT / SECURITY ]

No wallet functions.
No address generation.
No transaction signing.
No network settings.

---

## 3. Create from dice

Screen:

CREATE FROM DICE

Choose mnemonic length:

[ 12 WORDS ]

10 shakes
50 dice outcomes

[ 24 WORDS ]

20 shakes
100 dice outcomes

[ BACK ]

---

## 4. Dice ceremony introduction

After choosing 12 or 24 words:

PHYSICAL DICE ENTROPY

Use all five supplied dice.

Always enter them in the displayed order:

D1
D2
D3
D4
D5

Each die should have both:

- a fixed color
- a non-color identifier such as D1 through D5

Instructions:

1. Place all five dice in the tumbler.
2. Shake thoroughly.
3. Open the tumbler.
4. Enter each die result in D1 through D5 order.
5. Confirm the completed shake.
6. Repeat until complete.

Display:

12-word mode:
10 total shakes

24-word mode:
20 total shakes

[ BEGIN ]

[ BACK ]

---

## 5. Dice entry screen

Dice entry should occur one die at a time.

Example:

SHAKE 3 / 20

DIE D1
RED

What number is showing?

[ 1 ] [ 2 ] [ 3 ]

[ 4 ] [ 5 ] [ 6 ]

After selection, automatically advance to D2.

Repeat through D5.

The UI should clearly show progress:

D1 completed
D2 completed
D3 current
D4 pending
D5 pending

User should be able to correct an entry before confirming the shake.

Controls:

[ BACK ONE DIE ]

[ CANCEL CEREMONY ]

Large number buttons must be easy to tap accurately.

---

## 6. Confirm shake

After all five dice are entered:

SHAKE 3 / 20

Review:

D1: 4
D2: 2
D3: 6
D4: 1
D5: 5

[ CONFIRM SHAKE ]

[ EDIT ]

The five results are not appended to the canonical dice sequence until the user confirms the shake.

After confirmation:

SHAKE 3 SAVED

Advance to the next shake.

---

## 7. Undo previous shake

During an active ceremony, the user may undo the most recently confirmed shake.

Example:

[ UNDO LAST SHAKE ]

Confirmation:

REMOVE SHAKE 3?

D1: 4
D2: 2
D3: 6
D4: 1
D5: 5

[ YES, REMOVE ]

[ KEEP IT ]

Only the most recent confirmed shake may be removed.

---

## 8. Cancel ceremony

If the user chooses to cancel:

DESTROY THIS CEREMONY?

All entered dice results will be cleared.

No seed will be created.

[ DESTROY ]

[ GO BACK ]

Destroy must clear the active session.

Return to the home screen.

---

## 9. Dice entry complete

After:

12-word mode:
10 confirmed shakes / 50 outcomes

24-word mode:
20 confirmed shakes / 100 outcomes

Display:

PHYSICAL ENTROPY COMPLETE

All required dice outcomes have been entered.

Do not generate the mnemonic automatically.

First run the dice sanity checks.

---

## 10. Dice sanity warnings

If no warnings exist:

ENTROPY INPUT COMPLETE

No obvious input-pattern warnings detected.

This does not prove that the dice are perfectly fair.

[ GENERATE BIP39 MNEMONIC ]

[ REVIEW SHAKES ]

[ DESTROY CEREMONY ]

If one or more warnings exist:

ENTROPY WARNING

Example:

One die face never appeared.

or:

An unusually long repeated sequence was detected.

Explain clearly:

This warning does not prove the entropy is unsafe.
It indicates an unusual pattern in the entered dice results.

Options:

[ RESTART CEREMONY ]

[ REVIEW SHAKES ]

[ CONTINUE ANYWAY ]

Continuing must require a deliberate confirmation.

---

## 11. Generate confirmation

Before mnemonic generation:

GENERATE MNEMONIC?

Your physical dice sequence will now be converted using:

SHA-256
+
BIP39

The same exact dice input always produces the same mnemonic.

[ GENERATE ]

[ BACK ]

---

## 12. Mnemonic display

Display one mnemonic word at a time.

Example:

WORD 1 / 24

GRACE

BIP39 #0811

[ PREVIOUS ]

[ NEXT ]

The word should be the dominant element on screen.

The position number must always be visible.

Human-friendly BIP39 numbering may be displayed as:

0001 through 2048

Internal software indexing remains:

0 through 2047

These must not be confused.

At the final word:

WORD 24 / 24

[ PREVIOUS ]

[ FINISHED RECORDING ]

---

## 13. Backup review

After the user indicates all words are recorded:

BACKUP REVIEW

The device may perform a limited spot check.

Example:

What did you record for:

WORD 7?

User selects or enters the BIP39 word.

Repeat for several randomly or deterministically selected positions.

Important wording:

Do not state:

BACKUP VERIFIED

unless the complete backup has actually been checked.

Preferred wording for partial review:

SPOT CHECK PASSED

Your checked word positions matched.

This does not verify every written word.

Options:

[ REVIEW ALL WORDS AGAIN ]

[ DESTROY SESSION ]

---

## 14. Destroy session

Final destruction screen:

DESTROY ACTIVE SEED SESSION?

This will clear:

- dice results
- derived entropy
- mnemonic words
- temporary working data

The mnemonic will not be recoverable from this device.

[ DESTROY SESSION ]

[ GO BACK ]

After destruction:

SESSION DESTROYED

Return to home screen.

---

# Manual BIP39 Tools

## 15. Calculate final word

Home:

[ CALCULATE FINAL WORD ]

Choose:

[ 12-WORD PHRASE ]

Enter first 11 words.

or:

[ 24-WORD PHRASE ]

Enter first 23 words.

The device uses BIP39 word search and prefix resolution.

---

## 16. Word entry

Example:

WORD 4 / 23

Enter letters:

A B C D E F
...

As letters are entered, show matching BIP39 words.

Example:

AB

abandon
ability
able
about
above
...

After:

ABAN

only:

abandon

The user selects:

[ ABANDON ]

Then advance to the next word.

Exact words and unique prefixes are accepted internally.

Unknown or ambiguous entries must not be accepted as completed words.

---

## 17. Final checksum candidates

After 11 entered words:

VALID POSSIBLE WORD #12

Display all 128 valid candidates alphabetically.

Provide scrolling and prefix search.

After 23 entered words:

VALID POSSIBLE WORD #24

Display all 8 valid candidates alphabetically on one screen when practical.

Explain:

These words satisfy the BIP39 checksum for the entered prefix.

The device is not choosing entropy on the user's behalf.

---

# Validate BIP39 Phrase

## 18. Phrase validation

Home:

[ VALIDATE BIP39 PHRASE ]

Choose:

[ 12 WORDS ]

[ 24 WORDS ]

Enter the complete phrase using BIP39 word search.

After entry:

If valid:

VALID BIP39 PHRASE

All words are in the BIP39 English list.

Checksum is valid.

If invalid checksum:

INVALID BIP39 CHECKSUM

All entered words may be valid BIP39 words, but the checksum does not match.

If invalid word:

INVALID WORD

WORD 12 is not in the BIP39 English word list.

Never store the entered phrase after the validation session ends.

---

# Global UI Requirements

## 19. Touch targets

- large touch targets
- avoid tiny keyboard controls where possible
- six dice-number buttons should be especially large
- destructive actions must require confirmation
- accidental double taps should not duplicate input

## 20. Navigation

Every screen must have a predictable way to:

- go back
- cancel
- review
- destroy sensitive sessions

Navigation must never silently destroy seed material without warning.

Navigation must never silently save seed material.

## 21. Color

CryptoMachine visual direction:

- black / charcoal background
- orange accent
- white primary text

Dice colors may be shown in the UI.

Color must never be the only information used to identify a die.

## 22. Sensitive display behavior

Sensitive screens include:

- dice history
- entropy-related warnings
- mnemonic words
- manually entered BIP39 phrases

Production firmware should not:

- take screenshots
- log screen text
- persist screen history
- restore sensitive screens after reboot

## 23. Development principle

The UI must never implement independent cryptographic logic.

The UI calls the tested core functions.

Display code should not:

- generate entropy
- calculate BIP39 independently
- modify canonical dice input
- alter checksum behavior

The cryptographic core remains separate from the touchscreen layer.
