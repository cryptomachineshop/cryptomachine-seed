from core.reference_engine import load_wordlist
import hashlib


def words_to_prefix_bits(words):
    wordlist = load_wordlist()
    lookup = {word: index for index, word in enumerate(wordlist)}

    indexes = []

    for word in words:
        if word not in lookup:
            raise ValueError(f"Not a valid BIP39 English word: {word}")

        indexes.append(lookup[word])

    return "".join(f"{index:011b}" for index in indexes)


def checksum_candidates(words):
    """
    Given 11 or 23 valid BIP39 words, return every valid possible
    final checksum word.

    11 words -> 128 possible 12th words
    23 words -> 8 possible 24th words
    """

    if len(words) == 11:
        entropy_bits = 128
        checksum_bits = 4
        missing_entropy_bits = 7

    elif len(words) == 23:
        entropy_bits = 256
        checksum_bits = 8
        missing_entropy_bits = 3

    else:
        raise ValueError(
            "Checksum calculation requires exactly 11 or 23 words."
        )

    prefix = words_to_prefix_bits(words)

    wordlist = load_wordlist()
    candidates = []

    for value in range(1 << missing_entropy_bits):

        suffix = f"{value:0{missing_entropy_bits}b}"

        full_entropy_bits = prefix + suffix

        if len(full_entropy_bits) != entropy_bits:
            raise RuntimeError("Unexpected entropy bit length.")

        entropy = int(
            full_entropy_bits, 2
        ).to_bytes(entropy_bits // 8, "big")

        checksum_hash = hashlib.sha256(entropy).digest()

        checksum = "".join(
            f"{byte:08b}" for byte in checksum_hash
        )[:checksum_bits]

        final_word_bits = suffix + checksum
        final_index = int(final_word_bits, 2)

        candidates.append(wordlist[final_index])

    # BIP39 English wordlist is already alphabetical,
    # but explicitly sort for predictable UI behavior.
    return sorted(candidates)
