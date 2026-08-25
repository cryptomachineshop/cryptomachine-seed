from core.checksum_words import checksum_candidates
from core.mnemonic_validator import validate_mnemonic
from core.word_search import resolve_word, search_words


def resolve_entered_words(text):
    raw_words = text.strip().lower().split()

    resolved = []

    for position, entry in enumerate(raw_words, start=1):
        try:
            word = resolve_word(entry)

        except ValueError as exc:
            print()
            print(f"Problem with word {position}: {entry}")
            print(exc)

            try:
                matches = search_words(entry, limit=12)
            except ValueError:
                matches = []

            if matches:
                print()
                print("Possible matches:")
                for match in matches:
                    print(" -", match)

            raise

        resolved.append(word)

    return resolved


def paginate_words(words, page_size=16):
    total = len(words)

    for start in range(0, total, page_size):
        page = words[start:start + page_size]

        print()
        print(
            f"Candidates {start + 1}-"
            f"{start + len(page)} of {total}"
        )
        print("-" * 40)

        for number, word in enumerate(
            page,
            start=start + 1,
        ):
            print(
                f"{number:03d}. {word.upper()}"
            )

        if start + page_size < total:
            choice = input(
                "Press Enter for more, or Q to stop: "
            ).strip().lower()

            if choice == "q":
                break


def calculate_final_word():
    print()
    print("=" * 50)
    print("CALCULATE FINAL BIP39 WORD")
    print("=" * 50)
    print()
    print("1. 12-word phrase")
    print("2. 24-word phrase")
    print()

    choice = input("> ").strip()

    if choice == "1":
        expected = 11
        final_position = 12

    elif choice == "2":
        expected = 23
        final_position = 24

    else:
        print("Invalid selection.")
        return

    print()
    print(
        f"Enter the first {expected} BIP39 words "
        "on one line."
    )
    print()
    print(
        "Full words or unique prefixes may be used."
    )
    print()

    text = input("> ")

    try:
        words = resolve_entered_words(text)
    except ValueError:
        return

    if len(words) != expected:
        print()
        print(
            f"Expected exactly {expected} words, "
            f"received {len(words)}."
        )
        return

    candidates = checksum_candidates(words)

    print()
    print("=" * 50)
    print(
        f"VALID POSSIBLE WORD #{final_position}"
    )
    print("=" * 50)
    print()

    print(
        f"{len(candidates)} valid checksum "
        "candidate(s) found."
    )

    paginate_words(candidates)


def validate_phrase():
    print()
    print("=" * 50)
    print("VALIDATE BIP39 PHRASE")
    print("=" * 50)
    print()

    print(
        "Enter a complete 12-word or 24-word "
        "BIP39 phrase on one line."
    )
    print()
    print(
        "Full words or unique prefixes may be used."
    )
    print()

    text = input("> ")

    try:
        words = resolve_entered_words(text)
    except ValueError:
        return

    if len(words) not in (12, 24):
        print()
        print(
            "Phrase must contain exactly "
            "12 or 24 words."
        )
        return

    valid, reason = validate_mnemonic(words)

    print()

    if valid:
        print("=" * 50)
        print("VALID BIP39 PHRASE")
        print("=" * 50)
        print()
        print(
            "All words are valid and the "
            "BIP39 checksum matches."
        )

    else:
        print("=" * 50)
        print("INVALID BIP39 PHRASE")
        print("=" * 50)
        print()
        print(reason)


def main():
    while True:
        print()
        print("=" * 50)
        print("CRYPTOMACHINE BIP39 TOOLS")
        print("PC DEVELOPMENT SIMULATOR")
        print("=" * 50)
        print()
        print("TEST USE ONLY")
        print()
        print("1. Calculate final checksum word")
        print("2. Validate complete BIP39 phrase")
        print("Q. Quit")
        print()

        choice = input("> ").strip().lower()

        if choice == "1":
            calculate_final_word()

        elif choice == "2":
            validate_phrase()

        elif choice == "q":
            return

        else:
            print("Invalid selection.")


if __name__ == "__main__":
    main()
