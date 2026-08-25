from tools.seed_ceremony_sim import run_ceremony
from tools.bip39_tools_sim import (
    calculate_final_word,
    validate_phrase,
)


def create_from_dice():
    while True:
        print()
        print("=" * 50)
        print("CREATE FROM PHYSICAL DICE")
        print("=" * 50)
        print()
        print("1. 12 words")
        print("   10 shakes / 50 dice outcomes")
        print()
        print("2. 24 words")
        print("   20 shakes / 100 dice outcomes")
        print()
        print("B. Back")
        print()

        choice = input("> ").strip().lower()

        if choice == "1":
            run_ceremony(12)
            return

        if choice == "2":
            run_ceremony(24)
            return

        if choice == "b":
            return

        print("Invalid selection.")


def show_about():
    print()
    print("=" * 50)
    print("ABOUT / SECURITY")
    print("=" * 50)
    print()
    print("CryptoMachine Seed Tool")
    print()
    print("Development simulator only.")
    print()
    print("Design principles:")
    print()
    print("- Physical dice provide seed entropy")
    print("- Deterministic SHA-256 processing")
    print("- Standard BIP39 mnemonic generation")
    print("- 12-word and 24-word support")
    print("- Manual checksum-word calculation")
    print("- Complete BIP39 checksum validation")
    print("- No device-generated entropy")
    print("- No hidden salts or secret inputs")
    print("- Production target has no Wi-Fi or Bluetooth")
    print("- Production firmware will be open source")
    print()
    print(
        "Do not use this Windows simulator to create "
        "a seed that protects real funds."
    )
    print()

    input("Press Enter to return to the home screen...")


def main():
    while True:
        print()
        print("=" * 50)
        print("CRYPTOMACHINE")
        print("SEED TOOL")
        print("=" * 50)
        print()
        print("PC DEVELOPMENT SIMULATOR")
        print("TEST USE ONLY")
        print()
        print("1. CREATE FROM DICE")
        print("2. CALCULATE FINAL WORD")
        print("3. VALIDATE BIP39 PHRASE")
        print("4. ABOUT / SECURITY")
        print("Q. QUIT")
        print()

        choice = input("> ").strip().lower()

        if choice == "1":
            create_from_dice()

        elif choice == "2":
            calculate_final_word()

        elif choice == "3":
            validate_phrase()

        elif choice == "4":
            show_about()

        elif choice == "q":
            print()
            print("Goodbye.")
            return

        else:
            print("Invalid selection.")


if __name__ == "__main__":
    main()
