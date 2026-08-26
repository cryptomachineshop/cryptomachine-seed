#include "bip39.h"
#include "bip39_wordlist.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_wordlist() {
    const auto& words = cryptomachine::kBip39EnglishWordlist;

    check(
        words.size() == cryptomachine::kBip39WordlistSize,
        "BIP39 English list must contain exactly 2048 words"
    );

    const std::set<std::string_view> unique_words(
        words.begin(),
        words.end()
    );

    check(
        unique_words.size() == cryptomachine::kBip39WordlistSize,
        "BIP39 English list must contain 2048 unique words"
    );

    check(
        words.front() == "abandon",
        "BIP39 word index 0 must be abandon"
    );

    check(
        words.back() == "zoo",
        "BIP39 word index 2047 must be zoo"
    );
}

void test_zero_entropy_128() {
    const std::array<std::uint8_t, 16> entropy{};

    cryptomachine::Bip39Mnemonic mnemonic;

    check(
        cryptomachine::bip39_entropy_to_mnemonic(
            entropy,
            mnemonic
        ),
        "128-bit zero entropy must convert successfully"
    );

    check(
        mnemonic.word_count == 12,
        "128-bit entropy must produce 12 words"
    );

    const std::string actual =
        cryptomachine::bip39_mnemonic_to_string(
            mnemonic,
            cryptomachine::kBip39EnglishWordlist
        );

    const std::string expected =
        "abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon abandon abandon about";

    check(
        actual == expected,
        "official 128-bit zero-entropy BIP39 vector"
    );
}

void test_zero_entropy_256() {
    const std::array<std::uint8_t, 32> entropy{};

    cryptomachine::Bip39Mnemonic mnemonic;

    check(
        cryptomachine::bip39_entropy_to_mnemonic(
            entropy,
            mnemonic
        ),
        "256-bit zero entropy must convert successfully"
    );

    check(
        mnemonic.word_count == 24,
        "256-bit entropy must produce 24 words"
    );

    const std::string actual =
        cryptomachine::bip39_mnemonic_to_string(
            mnemonic,
            cryptomachine::kBip39EnglishWordlist
        );

    const std::string expected =
        "abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon abandon abandon art";

    check(
        actual == expected,
        "official 256-bit zero-entropy BIP39 vector"
    );
}

void test_all_ff_entropy_128() {
    std::array<std::uint8_t, 16> entropy{};
    entropy.fill(0xff);

    cryptomachine::Bip39Mnemonic mnemonic;

    check(
        cryptomachine::bip39_entropy_to_mnemonic(
            entropy,
            mnemonic
        ),
        "128-bit FF entropy must convert successfully"
    );

    const std::string actual =
        cryptomachine::bip39_mnemonic_to_string(
            mnemonic,
            cryptomachine::kBip39EnglishWordlist
        );

    const std::string expected =
        "zoo zoo zoo zoo zoo zoo "
        "zoo zoo zoo zoo zoo wrong";

    check(
        actual == expected,
        "official 128-bit FF BIP39 vector"
    );
}

void test_invalid_entropy_sizes() {
    constexpr std::array<std::size_t, 6> invalid_sizes{
        0, 1, 15, 17, 31, 33
    };

    for (const std::size_t size : invalid_sizes) {
        std::vector<std::uint8_t> entropy(size);

        cryptomachine::Bip39Mnemonic mnemonic;

        check(
            !cryptomachine::bip39_entropy_to_mnemonic(
                entropy,
                mnemonic
            ),
            "invalid entropy size must be rejected: " +
                std::to_string(size)
        );

        check(
            mnemonic.word_count == 0,
            "failed conversion must leave mnemonic empty"
        );
    }
}

}  // namespace

int main() {
    test_wordlist();
    test_zero_entropy_128();
    test_zero_entropy_256();
    test_all_ff_entropy_128();
    test_invalid_entropy_sizes();

    if (failures != 0) {
        std::cerr
            << failures
            << " BIP39 test(s) failed.\n";

        return 1;
    }

    std::cout << "All C++ BIP39 tests passed.\n";
    return 0;
}