from core.dice_sanity import analyze_five_dice
from core.seed_ceremony import SeedCeremony
from core.reference_engine import load_wordlist


def show_shakes(ceremony):
    shakes = ceremony.session.get_shakes()

    print()
    print("=" * 50)
    print("RECORDED SHAKES")
    print("=" * 50)

    if not shakes:
        print()
        print("No confirmed shakes yet.")
        return

    print()

    for index, shake in enumerate(shakes, start=1):
        print(
            f"{index:02d}. "
            f"D1={shake[0]} "
            f"D2={shake[1]} "
            f"D3={shake[2]} "
            f"D4={shake[3]} "
            f"D5={shake[4]}"
        )


def show_sanity_report(sanity):
    aggregate = sanity["aggregate"]

    print()
    print("=" * 50)
    print("DICE SANITY CHECK")
    print("=" * 50)
    print()

    print("Combined face counts:")
    print(aggregate["counts"])
    print()

    print(
        "Longest combined repeated-face run:",
        aggregate["longest_run"],
    )

    print()
    print("Per-die counts:")
    print()

    for die_name in (
        "D1",
        "D2",
        "D3",
        "D4",
        "D5",
    ):
        die = sanity["per_die"][die_name]

        print(
            f"{die_name}: "
            f"{die['counts']} "
            f"(longest run: {die['longest_run']})"
        )

    print()

    if sanity["warnings"]:
        print("=" * 50)
        print("WARNING")
        print("=" * 50)
        print()

        for warning in sanity["warnings"]:
            print(f"- {warning}")

        print()
        print(
            "These checks can identify obvious patterns, "
            "but they cannot prove that your dice are fair "
            "or that the recorded outcomes are random."
        )

    else:
        print("No gross dice patterns were detected.")
        print()
        print(
            "This does NOT prove that the dice are fair "
            "or that the outcomes are random."
        )


def show_words(mnemonic):
    words = mnemonic.split()
    wordlist = load_wordlist()

    print()
    print("=" * 50)
    print("RECORD YOUR BIP39 MNEMONIC")
    print("=" * 50)

    for position, word in enumerate(words, start=1):
        bip39_number = wordlist.index(word) + 1

        print()
        print("-" * 50)
        print(
            f"WORD {position} OF {len(words)}"
        )
        print()
        print(word.upper())
        print()
        print(
            f"BIP39 #{bip39_number:04d}"
        )
        print("-" * 50)

        if position < len(words):
            input(
                "Press Enter for NEXT word..."
            )

    print()
    input(
        "Press Enter to review the complete "
        "word list..."
    )


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


def run_ceremony(word_count=None):
    if word_count is None:
        print()
        print("=" * 50)
        print("CREATE FROM PHYSICAL DICE")
        print("=" * 50)
        print()
        print("1. 12 words")
        print("2. 24 words")
        print()

        choice = input("> ").strip()

        if choice == "1":
            word_count = 12

        elif choice == "2":
            word_count = 24

        else:
            print("Invalid selection.")
            return

    if word_count not in (12, 24):
        raise ValueError(
            "word_count must be 12 or 24"
        )

    ceremony = SeedCeremony(word_count)

    print()
    print("=" * 50)
    print(
        f"{word_count}-WORD DICE CEREMONY"
    )
    print("=" * 50)
    print()
    print("Use five physical D6 dice.")
    print()
    print(
        "Enter each shake as five digits "
        "in fixed D1-D5 order."
    )
    print()
    print("Example:")
    print("62415")
    print()
    print(
        f"Required shakes: "
        f"{ceremony.session.total_shakes}"
    )
    print()
    print("Commands:")
    print("U = undo last confirmed shake")
    print("R = review confirmed shakes")
    print("Q = cancel and destroy session")
    print()

    while not ceremony.session.complete:
        shake_number = (
            ceremony.session.shake_count + 1
        )

        total = ceremony.session.total_shakes

        entry = input(
            f"Shake {shake_number}/{total} "
            "[D1D2D3D4D5]: "
        ).strip().lower()

        if entry == "u":
            removed = (
                ceremony.session.undo_last_shake()
            )

            if removed is None:
                print("Nothing to undo.")
            else:
                print(
                    "Removed:",
                    "".join(removed),
                )

            continue

        if entry == "r":
            show_shakes(ceremony)
            continue

        if entry == "q":
            ceremony.destroy_session()

            print()
            print("Test session destroyed.")
            return

        if (
            len(entry) != 5
            or any(
                face not in "123456"
                for face in entry
            )
        ):
            print(
                "Enter exactly five digits, "
                "using only 1 through 6."
            )
            continue

        print()
        print("Review this shake:")
        print()
        print(f"D1: {entry[0]}")
        print(f"D2: {entry[1]}")
        print(f"D3: {entry[2]}")
        print(f"D4: {entry[3]}")
        print(f"D5: {entry[4]}")
        print()

        confirm = input(
            "Confirm shake? [Y/N]: "
        ).strip().lower()

        if confirm != "y":
            print("Shake not recorded.")
            continue

        ceremony.add_shake(list(entry))

    print()
    print("=" * 50)
    print("DICE ENTRY COMPLETE")
    print("=" * 50)
    print()
    print(
        f"{ceremony.session.shake_count} "
        "shakes recorded."
    )
    print(
        f"{len("".join("".join(shake) for shake in ceremony.session.get_shakes()))} "
        "outcomes recorded."
    )
    print()
    print(
        "The mnemonic has NOT been generated yet."
    )

    canonical = "".join("".join(shake) for shake in ceremony.session.get_shakes())
    sanity = analyze_five_dice(canonical)

    show_sanity_report(sanity)

    if sanity["warnings"]:
        print()
        print("Options:")
        print()
        print("C = continue anyway")
        print("R = review recorded shakes")
        print("X = restart ceremony")
        print("Q = cancel and destroy")
        print()

        while True:
            choice = input("> ").strip().lower()

            if choice == "r":
                show_shakes(ceremony)
                continue

            if choice == "x":
                ceremony.destroy_session()
                print()
                print("Restarting ceremony...")
                return run_ceremony(word_count)

            if choice == "q":
                ceremony.destroy_session()
                print()
                print("Test session destroyed.")
                return

            if choice == "c":
                break

            print("Invalid selection.")

    print()
    print("=" * 50)
    print("READY TO GENERATE")
    print("=" * 50)
    print()
    print(
        "The recorded physical dice results "
        "will now be converted into a "
        "standard BIP39 mnemonic."
    )
    print()
    print("No hidden salt.")
    print("No timestamp.")
    print("No device-generated randomness.")
    print()

    confirm = input(
        "Generate mnemonic? [Y/N]: "
    ).strip().lower()

    if confirm != "y":
        ceremony.destroy_session()

        print()
        print("Generation cancelled.")
        print("Test session destroyed.")
        return

    result = ceremony.generate()

    show_words(result["mnemonic"])
    show_final_review(result["mnemonic"])

    print()
    input(
        "Press Enter to destroy this "
        "test session..."
    )

    ceremony.destroy_session()

    print()
    print("Session destroyed.")


def main():
    run_ceremony()


if __name__ == "__main__":
    main()

