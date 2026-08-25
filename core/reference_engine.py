import hashlib
from pathlib import Path


WORDLIST_PATH = Path(__file__).with_name("bip39_english.txt")


def load_wordlist():
    words = WORDLIST_PATH.read_text(encoding="utf-8").splitlines()

    if len(words) != 2048:
        raise ValueError(
            f"Expected 2048 BIP39 words, found {len(words)}"
        )

    return words


def validate_dice_string(dice):
    if not dice:
        raise ValueError("Dice input cannot be empty.")

    invalid = [char for char in dice if char not in "123456"]

    if invalid:
        raise ValueError(
            "Dice input may contain only digits 1 through 6."
        )


def dice_to_entropy(dice, word_count):
    validate_dice_string(dice)

    digest = hashlib.sha256(dice.encode("ascii")).digest()

    if word_count == 12:
        return digest[:16]

    if word_count == 24:
        return digest

    raise ValueError("word_count must be 12 or 24.")


def entropy_to_mnemonic(entropy):
    entropy_bits_length = len(entropy) * 8

    if entropy_bits_length not in (128, 256):
        raise ValueError(
            "Entropy must be exactly 128 or 256 bits."
        )

    wordlist = load_wordlist()

    checksum_length = entropy_bits_length // 32
    checksum_hash = hashlib.sha256(entropy).digest()

    entropy_bits = "".join(
        f"{byte:08b}" for byte in entropy
    )

    checksum_bits = "".join(
        f"{byte:08b}" for byte in checksum_hash
    )[:checksum_length]

    combined_bits = entropy_bits + checksum_bits

    indexes = [
        int(combined_bits[i:i + 11], 2)
        for i in range(0, len(combined_bits), 11)
    ]

    return " ".join(
        wordlist[index] for index in indexes
    )


def dice_to_mnemonic(dice, word_count):
    entropy = dice_to_entropy(dice, word_count)
    return entropy_to_mnemonic(entropy)
