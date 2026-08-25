from core.checksum_words import checksum_candidates


WORDS_11 = (
    "abandon abandon abandon abandon abandon abandon "
    "abandon abandon abandon abandon abandon"
).split()

WORDS_23 = (
    "abandon abandon abandon abandon abandon abandon "
    "abandon abandon abandon abandon abandon abandon "
    "abandon abandon abandon abandon abandon abandon "
    "abandon abandon abandon abandon abandon"
).split()


candidates_12 = checksum_candidates(WORDS_11)
candidates_24 = checksum_candidates(WORDS_23)


print("12-word mode:")
print("Candidate count:", len(candidates_12))
print("First 10:", candidates_12[:10])
print()

print("24-word mode:")
print("Candidate count:", len(candidates_24))
print("Candidates:")
for word in candidates_24:
    print(" -", word)

print()


if len(candidates_12) != 128:
    raise SystemExit(
        f"FAIL: Expected 128 possible 12th words, got {len(candidates_12)}"
    )

if len(candidates_24) != 8:
    raise SystemExit(
        f"FAIL: Expected 8 possible 24th words, got {len(candidates_24)}"
    )

if "about" not in candidates_12:
    raise SystemExit(
        "FAIL: Expected known valid 12-word checksum candidate 'about'."
    )

if "art" not in candidates_24:
    raise SystemExit(
        "FAIL: Expected known valid 24-word checksum candidate 'art'."
    )

print("PASS: Checksum candidate counts and known vectors are correct.")
