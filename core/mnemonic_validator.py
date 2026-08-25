import hashlib

from core.reference_engine import load_wordlist


def validate_mnemonic(words):
    """
    Validate a 12-word or 24-word BIP39 English mnemonic.

    Returns:
        (True, "valid") when the mnemonic and checksum are valid.
        (False, reason) when invalid.
    """

    if isinstance(words, str):
        words = words.strip().split()

    if len(words) not in (12, 24):
        return False, "Mnemonic must contain exactly 12 or 24 words."

    wordlist = load_wordlist()
    lookup = {word: index for index, word in enumerate(wordlist)}

    indexes = []

    for position, word in enumerate(words, start=1):
        if word not in lookup:
            return False, f"Word {position} is not in the BIP39 English word list: {word}"

        indexes.append(lookup[word])

    combined_bits = "".join(
        f"{index:011b}" for index in indexes
    )

    if len(words) == 12:
        entropy_length = 128
        checksum_length = 4
    else:
        entropy_length = 256
        checksum_length = 8

    entropy_bits = combined_bits[:entropy_length]
    supplied_checksum = combined_bits[
        entropy_length:entropy_length + checksum_length
    ]

    entropy = int(
        entropy_bits, 2
    ).to_bytes(entropy_length // 8, "big")

    checksum_hash = hashlib.sha256(entropy).digest()

    expected_checksum = "".join(
        f"{byte:08b}" for byte in checksum_hash
    )[:checksum_length]

    if supplied_checksum != expected_checksum:
        return False, "BIP39 checksum is invalid."

    return True, "valid"
