from core.mnemonic_validator import validate_mnemonic


VALID_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)

VALID_24 = (
    "eyebrow obvious such suggest poet seven breeze blame virtual frown "
    "dynamic donor harsh pigeon express broccoli easy apology scatter "
    "force recipe shadow claim radio"
)

INVALID_CHECKSUM_12 = (
    "abandon abandon abandon abandon abandon abandon "
    "abandon abandon abandon abandon abandon abandon"
)

INVALID_WORD_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify notabip39word"
)


tests = [
    ("Valid 12-word mnemonic", VALID_12, True),
    ("Valid 24-word mnemonic", VALID_24, True),
    ("Invalid 12-word checksum", INVALID_CHECKSUM_12, False),
    ("Invalid BIP39 word", INVALID_WORD_12, False),
]


for name, mnemonic, expected in tests:
    valid, reason = validate_mnemonic(mnemonic)

    print(name)
    print("Result:", valid)
    print("Reason:", reason)
    print()

    if valid != expected:
        raise SystemExit(f"FAIL: {name}")


print("PASS: BIP39 mnemonic validation tests passed.")
