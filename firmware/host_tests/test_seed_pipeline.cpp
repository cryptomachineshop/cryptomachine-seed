#include "bip39.h"
#include "dice_entropy.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
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

    for (const auto& word : words) {
        views.emplace_back(word);
    }

    return views;
}

std::string mnemonic_from_dice(
    std::string_view dice,
    std::size_t word_count,
    std::span<const std::string_view> wordlist
) {
    cryptomachine::DiceEntropy entropy;

    if (!cryptomachine::dice_to_entropy(
            dice,
            word_count,
            entropy
        )) {
        return {};
    }

    cryptomachine::Bip39Mnemonic mnemonic;

    const std::span<const std::uint8_t> entropy_span(
        entropy.bytes.data(),
        entropy.size
    );

    if (!cryptomachine::bip39_entropy_to_mnemonic(
            entropy_span,
            mnemonic
        )) {
        return {};
    }

    return cryptomachine::bip39_mnemonic_to_string(
        mnemonic,
        wordlist
    );
}

void test_12_word_pipeline(
    std::span<const std::string_view> wordlist
) {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    const std::string expected =
        "hole luggage safe present express tragic orbit shed "
        "switch metal identify path";

    const std::string actual =
        mnemonic_from_dice(
            dice,
            12,
            wordlist
        );

    check(
        actual == expected,
        "12-word full C++ seed pipeline"
    );
}

void test_24_word_pipeline(
    std::span<const std::string_view> wordlist
) {
    constexpr std::string_view dice =
        "12345612345612345612345612345612345612345612345612"
        "34561234561234561234561234561234561234561234561234";

    const std::string expected =
        "tornado cactus wheel picture target finish home neither "
        "trend picture shoulder endless deputy glide open oxygen "
        "another ability forum swear side alcohol devote random";

    const std::string actual =
        mnemonic_from_dice(
            dice,
            24,
            wordlist
        );

    check(
        actual == expected,
        "24-word full C++ seed pipeline"
    );
}

}  // namespace

int main() {
    const auto words = load_wordlist();

    check(
        words.size() == cryptomachine::kBip39WordlistSize,
        "BIP39 word list must contain 2048 words"
    );

    if (words.size() != cryptomachine::kBip39WordlistSize) {
        return 1;
    }

    const auto views = make_views(words);

    const std::span<const std::string_view> wordlist(
        views.data(),
        views.size()
    );

    test_12_word_pipeline(wordlist);
    test_24_word_pipeline(wordlist);

    if (failures != 0) {
        std::cerr
            << failures
            << " seed pipeline test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ seed pipeline tests passed.\n";

    return 0;
}