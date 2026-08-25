import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORDLIST_PATH = ROOT / "core" / "bip39_english.txt"

dice = "65515223131652132161133154444123616466443112153441"

expected_mnemonic = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)

words = WORDLIST_PATH.read_text(encoding="utf-8").splitlines()

if len(words) != 2048:
    raise SystemExit(f"FAIL: Expected 2048 BIP39 words, found {len(words)}")

# Dice string -> SHA-256
full_hash = hashlib.sha256(dice.encode("ascii")).digest()

# SeedSigner 12-word method:
# use first 16 bytes = 128 bits of entropy
entropy = full_hash[:16]

entropy_bit_length = len(entropy) * 8
checksum_length = entropy_bit_length // 32

checksum_hash = hashlib.sha256(entropy).digest()

entropy_bits = "".join(f"{byte:08b}" for byte in entropy)
checksum_bits = "".join(f"{byte:08b}" for byte in checksum_hash)[:checksum_length]

combined_bits = entropy_bits + checksum_bits

indexes = [
    int(combined_bits[i:i + 11], 2)
    for i in range(0, len(combined_bits), 11)
]

mnemonic = " ".join(words[index] for index in indexes)

print("Dice outcomes:", len(dice))
print("Full SHA-256:", full_hash.hex())
print("128-bit entropy:", entropy.hex())
print("Checksum bits:", checksum_bits)
print("Word count:", len(indexes))
print()
print("Mnemonic:")
print(mnemonic)
print()

if mnemonic == expected_mnemonic:
    print("PASS: 12-word BIP39 mnemonic matches SeedSigner reference vector.")
else:
    print("FAIL: Mnemonic does not match expected reference.")
    raise SystemExit(1)
