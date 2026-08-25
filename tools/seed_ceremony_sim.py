from core.dice_sanity import analyze_dice
from core.seed_ceremony import SeedCeremony
from core.reference_engine import load_wordlist


def show_shakes(ceremony):
    shakes = ceremony.session.get_shakes()

    print()
    print("CONFIRMED SHAKES")
    print("-" * 40)

    if not shakes:
        print("None")
    else:
        for number, shake in enumerate(shakes, start=1):
            print(
                f"{number:02d}: "
                + " ".join(shake)
            )

    print()


def show_words(mnemonic):
    words = mnemonic.split()
    wordlist = load_wordlist()

    indexes = {
        word: index + 1
        for index, word in enumerate(wordlist)
    }

    print()
    print("=" * 50)
    print("TEST MNEMONIC")
    print("=" * 50)
    print()

    for position, word in enumerate(words, start=1):
        print(
            f"WORD {position:02d} / {len(words)}"
        )

        print(
            f"{word.upper()}   "
            f"(BIP39 #{indexes[word]:04d})"
        )

        if position != len(words):
            input("Press Enter for next word...")

        print()

    print("=" * 50)


def show_final_review(mnemonic):
    words = mnemonic.split()

    print()
    print("=" * 50)
    print("FINAL MNEMONIC REVIEW")
    print("=" * 50)
    print()

    for position, word in enumerate(words, start=1):
        print(
            f"{position:02d}. {word.upper()}"
        )

    print()
    print("=" * 50)
    print()


def choose_word_count():
    while True:
        print()
        print("CREATE TEST SEED")
        print()
        print("1. 12 words")
        print("2. 24 words")
        print("Q. Quit")
        print()

        choice = input("> ").strip().lower()

        if choice == "1":
            return 12

        if choice == "2":
            return 24

        if choice == "q":
            return None

        print("Invalid selection.")


def run_ceremony(word_count):
    ceremony = SeedCeremony(word_count)

    print()
    print("=" * 50)
    print(
        f"{word_count}-WORD TEST CEREMONY"
    )
    print("=" * 50)
    print()

    print(
        f"Requires {ceremony.total_shakes} shakes "
        f"of five dice."
    )

    print()
    print("Enter each shake as five digits.")
    print("Example: 62415")
    print()

    print("Commands:")
    print("  U = undo last confirmed shake")
    print("  R = review confirmed shakes")
    print("  Q = destroy and quit")
    print()

    while not ceremony.complete:
        next_shake = ceremony.shake_count + 1

        print(
            f"SHAKE {next_shake} / "
            f"{ceremony.total_shakes}"
        )

        entry = input(
            "D1 D2 D3 D4 D5 > "
        ).strip().lower()

        if entry == "u":
            try:
                removed = ceremony.undo_last_shake()

                print(
                    "Removed:",
                    " ".join(removed),
                )

            except ValueError as exc:
                print(exc)

            print()
            continue

        if entry == "r":
            show_shakes(ceremony)
            continue

        if entry == "q":
            ceremony.destroy_session()

            print()
            print("TEST SESSION DESTROYED")

            return

        entry = entry.replace(" ", "")

        if len(entry) != 5:
            print(
                "Enter exactly five values "
                "from 1 through 6."
            )

            print()
            continue

        try:
            ceremony.add_shake(entry)

        except ValueError as exc:
            print(exc)
            print()
            continue

        print(
            f"Shake {ceremony.shake_count} saved."
        )

        print()

    dice = ceremony.session.canonical_dice_string()
    sanity = analyze_dice(dice)

    print()
    print("=" * 50)
    print("PHYSICAL ENTROPY INPUT COMPLETE")
    print("=" * 50)
    print()

    print(
        f"Outcomes collected: {len(dice)}"
    )

    print(
        "Face counts:",
        sanity["counts"],
    )

    print(
        "Longest identical run:",
        sanity["longest_run"],
    )

    print()

    if sanity["warnings"]:
        print("WARNINGS:")

        for warning in sanity["warnings"]:
            print(" -", warning)

        print()

        print(
            "These warnings do not prove that "
            "the entropy is unsafe."
        )

        print()

        confirm = input(
            "Type CONTINUE to generate anyway: "
        ).strip()

        if confirm != "CONTINUE":
            ceremony.destroy_session()

            print()
            print("TEST SESSION DESTROYED")

            return

    else:
        print(
            "No obvious input-pattern warnings detected."
        )

        print(
            "This does not prove the dice are perfectly fair."
        )

        print()

    choice = input(
        "Generate test mnemonic? (Y/N): "
    ).strip().lower()

    if choice != "y":
        ceremony.destroy_session()

        print()
        print("TEST SESSION DESTROYED")

        return

    result = ceremony.generate()

    show_words(result["mnemonic"])

    input(
        "Press Enter to review the complete word list..."
    )

    show_final_review(result["mnemonic"])

    input(
        "Press Enter to destroy this test session..."
    )

    ceremony.destroy_session()

    print()
    print("TEST SESSION DESTROYED")
    print()


def main():
    print()
    print("=" * 50)
    print("CRYPTOMACHINE SEED GENERATOR")
    print("PC DEVELOPMENT SIMULATOR")
    print("=" * 50)
    print()

    print("TEST USE ONLY")
    print()

    print(
        "Do not use this Windows development simulator "
        "to create a seed that will protect real funds."
    )

    word_count = choose_word_count()

    if word_count is None:
        return

    run_ceremony(word_count)


if __name__ == "__main__":
    main()
