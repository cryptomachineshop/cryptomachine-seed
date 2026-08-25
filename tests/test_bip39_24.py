import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORDLIST_PATH = ROOT / "core" / "bip39_english.txt"

dice = "655152231316521321611331544441236164664431121534415633526456254462245546236542364246312613322234612"

expected_mnemonic = (
    "eyebrow obvious such suggest poet seven breeze blame virtual frown "
    "dynamic donor harsh pigeon express broccoli easy apology scatter "
    "force recipe shadow claim radio"
)

words = WORDLIST_PATH.read_text(encoding="utf-8").splitlines()

if len(words) != 2048:
    raise SystemExit(f"FAIL: Expected 2048 BIP39 words, found {len(words)}")

# Dice string -> SHA-256 -> 256 bits of entropy
entropy = hashlib.sha256(dice.encode("ascii")).digest()

# BIP39 checksum length = entropy_bits / 32
entropy_bit_length = len(entropy) * 8
checksum_length = entropy_bit_length // 32

# Checksum is taken from SHA-256(entropy)
checksum_hash = hashlib.sha256(entropy).digest()

entropy_bits = "".join(f"{byte:08b}" for byte in entropy)
checksum_bits = "".join(f"{byte:08b}" for byte in checksum_hash)[:checksum_length]

combined_bits = entropy_bits + checksum_bits

# Split into 11-bit BIP39 word indexes
indexes = [
    int(combined_bits[i:i + 11], 2)
    for i in range(0, len(combined_bits), 11)
]

mnemonic = " ".join(words[index] for index in indexes)

print("Dice outcomes:", len(dice))
print("Entropy:", entropy.hex())
print("Checksum bits:", checksum_bits)
print("Word count:", len(indexes))
print()
print("Mnemonic:")
print(mnemonic)
print()

if mnemonic == expected_mnemonic:
    print("PASS: BIP39 mnemonic matches SeedSigner reference vector.")
else:
    print("FAIL: Mnemonic does not match expected reference.")
    raise SystemExit(1)
