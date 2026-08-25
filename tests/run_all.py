import subprocess
import sys


TEST_MODULES = [
    "tests.test_dice_hash",
    "tests.test_bip39_12",
    "tests.test_bip39_24",
    "tests.test_reference_engine",
    "tests.test_checksum_words",
    "tests.test_mnemonic_validator",
    "tests.test_dice_sanity",
    "tests.test_dice_policy",
    "tests.test_seed_engine",
    "tests.test_word_search",
    "tests.test_dice_session",
    "tests.test_seed_ceremony",
    "tests.test_ui_state_machine",
]


def main():
    for module in TEST_MODULES:
        print()
        print("=" * 60)
        print(f"RUNNING: {module}")
        print("=" * 60)

        result = subprocess.run(
            [sys.executable, "-m", module]
        )

        if result.returncode != 0:
            print()
            print(f"FAIL: {module}")
            sys.exit(result.returncode)

    print()
    print("=" * 60)
    print("PASS: ALL CRYPTOMACHINE CORE TESTS PASSED")
    print("=" * 60)


if __name__ == "__main__":
    main()
