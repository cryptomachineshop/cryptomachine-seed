import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

SOURCE = ROOT / "core" / "bip39_english.txt"
HEADER = ROOT / "firmware" / "core" / "bip39_wordlist.h"
SOURCE_CPP = ROOT / "firmware" / "core" / "bip39_wordlist.cpp"


def load_words():
    words = SOURCE.read_text(encoding="utf-8").splitlines()

    if len(words) != 2048:
        raise ValueError(
            f"Expected 2048 BIP39 words, found {len(words)}"
        )

    if len(set(words)) != 2048:
        raise ValueError(
            "BIP39 word list must contain 2048 unique words"
        )

    if words[0] != "abandon":
        raise ValueError(
            'Expected first BIP39 word to be "abandon"'
        )

    if words[-1] != "zoo":
        raise ValueError(
            'Expected final BIP39 word to be "zoo"'
        )

    return words


def generate_header():
    return """#pragma once

#include <array>
#include <string_view>

namespace cryptomachine {

extern const std::array<std::string_view, 2048>
    kBip39EnglishWordlist;

}  // namespace cryptomachine
"""


def generate_cpp(words):
    lines = [
        '#include "bip39_wordlist.h"',
        "",
        "namespace cryptomachine {",
        "",
        "const std::array<std::string_view, 2048>",
        "    kBip39EnglishWordlist{{",
    ]

    for word in words:
        lines.append(f'        "{word}",')

    lines.extend(
        [
            "    }};",
            "",
            "}  // namespace cryptomachine",
            "",
        ]
    )

    return "\n".join(lines)


def normalized_file_text(path):
    return path.read_text(
        encoding="utf-8"
    ).replace("\r\n", "\n")


def check_generated_file(path, expected):
    if not path.exists():
        print(
            f"FAIL: Missing generated file: "
            f"{path.relative_to(ROOT)}"
        )
        return False

    actual = normalized_file_text(path)

    if actual != expected:
        print(
            f"FAIL: Generated file is stale: "
            f"{path.relative_to(ROOT)}"
        )
        return False

    print(
        f"PASS: {path.relative_to(ROOT)}"
    )
    return True


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify generated files without modifying them.",
    )

    args = parser.parse_args()

    words = load_words()

    header_text = generate_header()
    cpp_text = generate_cpp(words)

    if args.check:
        header_ok = check_generated_file(
            HEADER,
            header_text,
        )

        cpp_ok = check_generated_file(
            SOURCE_CPP,
            cpp_text,
        )

        if not header_ok or not cpp_ok:
            raise SystemExit(1)

        print(
            "PASS: Embedded BIP39 word list is reproducible."
        )
        return

    HEADER.write_text(
        header_text,
        encoding="utf-8",
        newline="\n",
    )

    SOURCE_CPP.write_text(
        cpp_text,
        encoding="utf-8",
        newline="\n",
    )

    print(
        "Generated:",
        HEADER.relative_to(ROOT),
    )

    print(
        "Generated:",
        SOURCE_CPP.relative_to(ROOT),
    )

    print(
        f"Embedded {len(words)} BIP39 words."
    )


if __name__ == "__main__":
    main()