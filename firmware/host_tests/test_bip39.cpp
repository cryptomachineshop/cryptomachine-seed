#include "bip39.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::vector<std::string> load_wordlist() {
    const std::filesystem::path path =
        std::filesystem::path(CRYPTOMACHINE_SOURCE_DIR) /
        "core" /
        "bip39_english.txt";

    std::ifstream file(path);

    if (!file) {
        std::cerr
            << "Unable to open BIP39 word list: "
            << path
            << '\n';

        return {};
    }

    std::vector<std::string> words;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (!line.empty()) {
            words.push_back(line);
        }
    }

    return words;
}

std::vector<std::string_view> make_views(
    const std::vector<std::string>& words
) {
    std::vector<std::string_view> views;
    views.reserve(words.size());

    for (const std::string& word : words) {
        views.emplace_back(word);
    }

    return views;
}

void test_wordlist(
    const std::vector<std::string>& words
) {
    check(
        words.size() == cryptomachine::kBip39WordlistSize,
        "BIP39 English list must contain exactly 2048 words"
    );

    const std::set<std::string> unique_words(
        words.begin(),
        words.end()
    );

    check(
        unique_words.size() == cryptomachine::kBip39WordlistSize,
        "BIP39 English list must contain 2048 unique words"
    );

    if (words.size() == cryptomachine::kBip39WordlistSize) {
        check(
            words.front() == "abandon",
            "BIP39 word index 0 must be abandon"
        );

        check(
            words.back() == "zoo",
            "BIP39 word index 2047 must be zoo"
        );
    }
}

void test_zero_entropy_128(
    std::span<const std::string_view> wordlist
) {
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
            wordlist
        );

    const std::string expected =
        "abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon abandon abandon about";

    check(
        actual == expected,
        "official 128-bit zero-entropy BIP39 vector"
    );
}

void test_zero_entropy_256(
    std::span<const std::string_view> wordlist
) {
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
            wordlist
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

void test_all_ff_entropy_128(
    std::span<const std::string_view> wordlist
) {
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
            wordlist
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
    const std::vector<std::string> words = load_wordlist();

    test_wordlist(words);

    if (words.size() != cryptomachine::kBip39WordlistSize) {
        std::cerr << "Cannot continue BIP39 tests.\n";
        return 1;
    }

    const std::vector<std::string_view> word_views =
        make_views(words);

    const std::span<const std::string_view> wordlist(
        word_views.data(),
        word_views.size()
    );

    test_zero_entropy_128(wordlist);
    test_zero_entropy_256(wordlist);
    test_all_ff_entropy_128(wordlist);
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