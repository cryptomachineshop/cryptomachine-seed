import subprocess
import sys


TESTS = [
    "tests.test_dice_hash",
    "tests.test_bip39_12",
    "tests.test_bip39_24",
    "tests.test_reference_engine",
    "tests.test_checksum_words",
    "tests.test_mnemonic_validator",
    "tests.test_dice_sanity",
]

print("=" * 60)
print("CryptoMachine Seed Generator - Core Test Suite")
print("=" * 60)
print()


for test in TESTS:
    print(f"Running {test}...")
    print("-" * 60)

    result = subprocess.run(
        [sys.executable, "-m", test]
    )

    print()

    if result.returncode != 0:
        print("=" * 60)
        print(f"FAIL: {test}")
        print("=" * 60)
        raise SystemExit(result.returncode)


print("=" * 60)
print("PASS: ALL CRYPTOMACHINE CORE TESTS PASSED")
print("=" * 60)
