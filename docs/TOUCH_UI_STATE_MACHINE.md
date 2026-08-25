# CryptoMachine Seed Tool
# V1 Touchscreen State Machine

Status: DEVELOPMENT SPECIFICATION

Target hardware:
Waveshare RP2350-Touch-LCD-3.5
320 x 480 capacitive touchscreen

The touchscreen layer must never implement cryptographic logic.
It calls the tested core functions and displays results.

---

## 1. Global Design Rules

1. No network functionality.
2. No seed or mnemonic persistence.
3. No sensitive logging.
4. Sensitive session data exists only during an active ceremony.
5. Cancel, finish, reset, timeout, or restart must destroy sensitive session data.
6. Destructive actions require deliberate confirmation.
7. Generating a mnemonic must require an explicit user action.
8. Dice identities are D1 through D5.
9. Colors may assist identification but must never be the only identifier.
10. BIP39 display numbers shown to users are 0001 through 2048.
11. Internal BIP39 indexes remain 0 through 2047.
12. Duplicate mnemonic words are valid and must not trigger warnings.
13. The UI must clearly distinguish test/development builds from production firmware.

---

## 2. Top-Level States

BOOT
HOME
DICE_WORD_COUNT
DICE_INTRO
DICE_ENTRY
DICE_SHAKE_REVIEW
DICE_COMPLETE
DICE_SANITY_WARNING
DICE_GENERATE_CONFIRM
MNEMONIC_WORD_VIEW
MNEMONIC_FULL_REVIEW
SESSION_DESTROY_CONFIRM

FINAL_WORD_MODE
FINAL_WORD_ENTRY
FINAL_WORD_RESULTS

VALIDATE_PHRASE_ENTRY
VALIDATE_PHRASE_RESULT

ABOUT_SECURITY

---

## 3. BOOT

Purpose:
Initialize hardware and clear sensitive RAM before showing the application.

Actions:
- Initialize display.
- Initialize touchscreen.
- Initialize core services.
- Clear all sensitive session buffers.
- Do not automatically restore any previous seed session.

Transition:

BOOT
  -> HOME

---

## 4. HOME

Display:

CRYPTOMACHINE
SEED TOOL

Buttons:

[ CREATE FROM DICE ]

[ CALCULATE FINAL WORD ]

[ VALIDATE BIP39 PHRASE ]

[ ABOUT / SECURITY ]

Transitions:

CREATE FROM DICE
  -> DICE_WORD_COUNT

CALCULATE FINAL WORD
  -> FINAL_WORD_MODE

VALIDATE BIP39 PHRASE
  -> VALIDATE_PHRASE_ENTRY

ABOUT / SECURITY
  -> ABOUT_SECURITY

---

## 5. DICE_WORD_COUNT

Display:

CREATE FROM DICE

[ 12 WORDS ]
10 shakes
50 outcomes

[ 24 WORDS ]
20 shakes
100 outcomes

[ BACK ]

Transitions:

12 WORDS
  -> initialize DiceSession(12)
  -> DICE_INTRO

24 WORDS
  -> initialize DiceSession(24)
  -> DICE_INTRO

BACK
  -> HOME

---

## 6. DICE_INTRO

Purpose:
Explain the physical dice process before recording entropy.

Display:

USE FIVE PHYSICAL D6 DICE

Record each shake in this order:

D1
D2
D3
D4
D5

Shake the dice thoroughly before
entering each result.

Buttons:

[ BEGIN ]

[ CANCEL ]

Transitions:

BEGIN
  -> DICE_ENTRY

CANCEL
  -> destroy active session
  -> HOME

---

## 7. DICE_ENTRY

Display example:

SHAKE 4 OF 20

D1

[ 1 ] [ 2 ]
[ 3 ] [ 4 ]
[ 5 ] [ 6 ]

Current shake:

D1: 4
D2: 2
D3: _
D4: _
D5: _

Behavior:

- User enters one die at a time.
- Entry order is D1 -> D2 -> D3 -> D4 -> D5.
- After each number selection, advance automatically to the next die.
- Results are not committed to the entropy session until the full shake is confirmed.

When D5 has been entered:

DICE_ENTRY
  -> DICE_SHAKE_REVIEW

Additional buttons:

[ CANCEL ]

If at least one prior shake exists:

[ UNDO LAST SHAKE ]

CANCEL
  -> SESSION_DESTROY_CONFIRM

UNDO LAST SHAKE
  -> remove last confirmed shake
  -> DICE_ENTRY

---

## 8. DICE_SHAKE_REVIEW

Display:

CONFIRM SHAKE 4

D1   4
D2   2
D3   6
D4   1
D5   3

Buttons:

[ CONFIRM SHAKE ]

[ RE-ENTER ]

[ CANCEL ]

CONFIRM SHAKE:
- Append exactly five outcomes to the active DiceSession.

If more shakes are required:

DICE_SHAKE_REVIEW
  -> DICE_ENTRY

If required shake count has been reached:

DICE_SHAKE_REVIEW
  -> DICE_COMPLETE

RE-ENTER:
- Discard only the unconfirmed current shake.
- Return to DICE_ENTRY.

CANCEL:
  -> SESSION_DESTROY_CONFIRM

---

## 9. DICE_COMPLETE

Display:

DICE ENTRY COMPLETE

20 shakes recorded
100 outcomes recorded

The mnemonic has NOT been
generated yet.

Buttons:

[ REVIEW ]

[ CONTINUE ]

[ CANCEL ]

REVIEW:
- Show confirmed shakes without revealing any generated mnemonic.

CONTINUE:
- Run dice sanity analysis.

If no warnings:

DICE_COMPLETE
  -> DICE_GENERATE_CONFIRM

If warnings exist:

DICE_COMPLETE
  -> DICE_SANITY_WARNING

CANCEL:
  -> SESSION_DESTROY_CONFIRM

---

## 10. DICE_SANITY_WARNING

Purpose:
Warn about obvious dice-entry patterns without claiming to certify randomness.

Example:

DICE CHECK WARNING

The entered results contain
patterns that deserve review.

Possible warnings:

- One or more faces never appeared.
- An unusually long repeated run was detected.
- One face appeared unusually often.
- A gross per-die pattern was detected.

Important wording:

THIS CHECK CANNOT PROVE THAT
YOUR DICE ARE FAIR OR RANDOM.

Buttons:

[ REVIEW DICE ]

[ CONTINUE ANYWAY ]

[ RESTART ]

[ CANCEL ]

REVIEW DICE
  -> show recorded shakes
  -> DICE_SANITY_WARNING

CONTINUE ANYWAY
  -> DICE_GENERATE_CONFIRM

RESTART
  -> destroy current session
  -> initialize new DiceSession with same word count
  -> DICE_INTRO

CANCEL
  -> SESSION_DESTROY_CONFIRM

---

## 11. DICE_GENERATE_CONFIRM

Display:

READY TO GENERATE

The recorded physical dice results
will now be converted into a
standard BIP39 mnemonic.

No hidden salt.
No timestamp.
No device-generated randomness.

Button:

[ GENERATE MNEMONIC ]

Secondary:

[ BACK ]

[ CANCEL ]

GENERATE MNEMONIC:
- Call the core seed-generation function exactly once.
- Store result only in active sensitive RAM.
- Lock further dice editing.

Transition:

DICE_GENERATE_CONFIRM
  -> MNEMONIC_WORD_VIEW

BACK
  -> DICE_COMPLETE

CANCEL
  -> SESSION_DESTROY_CONFIRM

---

## 12. MNEMONIC_WORD_VIEW

Purpose:
Allow careful handwritten recording one word at a time.

Display example:

WORD 7 OF 24

HOME

BIP39 #0865

[ NEXT ]

For words after #1:

[ PREVIOUS ]

Rules:

- One mnemonic word is emphasized at a time.
- Show human BIP39 number 0001 through 2048.
- Duplicate words are allowed.
- Do not automatically advance.
- User must deliberately press NEXT.

After the final word:

MNEMONIC_WORD_VIEW
  -> MNEMONIC_FULL_REVIEW

---

## 13. MNEMONIC_FULL_REVIEW

Purpose:
Show the entire phrase one final time before session destruction.

12-word phrases may fit on one page.

24-word phrases may use two pages if required.

Example:

FINAL MNEMONIC REVIEW

01. WORD
02. WORD
03. WORD
...
24. WORD

Buttons:

[ PREVIOUS PAGE ]
when required

[ NEXT PAGE ]
when required

Final page:

[ FINISH ]

FINISH:
  -> SESSION_DESTROY_CONFIRM

The full mnemonic must remain available until the user deliberately finishes.

---

## 14. SESSION_DESTROY_CONFIRM

Display:

DESTROY THIS SESSION?

The mnemonic and dice session
will be cleared from working memory.

Make sure your backup is complete.

Buttons:

[ DESTROY SESSION ]

[ GO BACK ]

DESTROY SESSION:
- Zero sensitive buffers.
- Destroy mnemonic data.
- Destroy entropy data.
- Destroy dice-session data.
- Clear sensitive UI text.
- Return to HOME.

GO BACK:
- Return to the immediately previous sensitive screen.

---

## 15. FINAL_WORD_MODE

Display:

CALCULATE FINAL WORD

[ 12-WORD PHRASE ]
Enter first 11 words

[ 24-WORD PHRASE ]
Enter first 23 words

[ BACK ]

Transitions:

12-WORD
  -> configure expected input count = 11
  -> FINAL_WORD_ENTRY

24-WORD
  -> configure expected input count = 23
  -> FINAL_WORD_ENTRY

BACK
  -> HOME

---

## 16. FINAL_WORD_ENTRY

Purpose:
Enter existing BIP39 words.

Features:

- Search BIP39 English word list.
- Accept full exact words.
- Accept unique prefixes.
- Display matching candidates.
- Show word position clearly.
- Allow previous-word correction.

Example:

WORD 5 OF 23

[ search field ]

Matches:

about
above
absent
absorb

Buttons:

[ SELECT ]

[ PREVIOUS ]

[ CANCEL ]

After all required words are entered:

FINAL_WORD_ENTRY
  -> calculate valid checksum candidates
  -> FINAL_WORD_RESULTS

---

## 17. FINAL_WORD_RESULTS

For 11 entered words:

Display 128 valid possible word #12 candidates.

For 23 entered words:

Display 8 valid possible word #24 candidates.

Candidates must be sorted alphabetically.

Example:

VALID POSSIBLE WORD #24

1. ART
2. DIESEL
3. FALSE
4. KITE
5. ORGAN
6. READY
7. SURFACE
8. TROUBLE

Buttons:

[ BACK ]

[ HOME ]

No claim should be made that one candidate is the user's correct word unless the missing entropy is independently known.

---

## 18. VALIDATE_PHRASE_ENTRY

Purpose:
Validate an existing complete BIP39 phrase.

User chooses:

[ 12 WORDS ]

[ 24 WORDS ]

Then enters each word using the same BIP39 word-search interface.

After final word:

VALIDATE_PHRASE_ENTRY
  -> run BIP39 checksum validation
  -> VALIDATE_PHRASE_RESULT

---

## 19. VALIDATE_PHRASE_RESULT

Valid result:

VALID BIP39 PHRASE

All words are valid and the
BIP39 checksum matches.

Invalid result:

INVALID BIP39 PHRASE

The entered words do not produce
a valid BIP39 checksum.

Buttons:

[ REVIEW WORDS ]

[ START OVER ]

[ HOME ]

Sensitive entered phrase data must be destroyed when leaving this workflow.

---

## 20. ABOUT_SECURITY

Display concise security architecture information.

Topics:

- Physical dice provide entropy.
- SHA-256 processing is deterministic.
- Standard BIP39 generation.
- No hidden salt.
- No timestamp input.
- No device-generated entropy.
- No Wi-Fi or Bluetooth hardware on target board.
- No seed storage by design.
- Open-source production firmware.
- Firmware verification information.

Buttons:

[ BACK ]

BACK
  -> HOME

---

## 21. Sensitive State Classification

Sensitive states:

DICE_ENTRY
DICE_SHAKE_REVIEW
DICE_COMPLETE
DICE_SANITY_WARNING
DICE_GENERATE_CONFIRM
MNEMONIC_WORD_VIEW
MNEMONIC_FULL_REVIEW
SESSION_DESTROY_CONFIRM
FINAL_WORD_ENTRY
FINAL_WORD_RESULTS
VALIDATE_PHRASE_ENTRY
VALIDATE_PHRASE_RESULT

Non-sensitive states:

BOOT
HOME
DICE_WORD_COUNT
DICE_INTRO
FINAL_WORD_MODE
ABOUT_SECURITY

Any transition from a sensitive workflow back to HOME must clear that workflow's sensitive memory.

---

## 22. Core/UI Separation

UI code may:

- Draw screens.
- Accept touch input.
- Navigate between states.
- Display core results.
- Request core operations.

UI code must not:

- Implement SHA-256.
- Construct BIP39 checksums.
- Convert entropy directly into mnemonic words.
- Decide checksum candidates independently.
- Perform cryptographic shortcuts.

All cryptographic operations must remain inside the reviewed core implementation.

---

## 23. V1 State Machine Principle

Each screen should have:

- One clear purpose.
- One obvious primary action.
- Minimal sensitive information.
- Explicit navigation.
- No hidden state transitions.
- No automatic mnemonic generation.
- No automatic session destruction.

The state machine should be deterministic and testable independently of the touchscreen hardware.
