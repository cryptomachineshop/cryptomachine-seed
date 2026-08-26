#include "bip39.h"
#include "bip39_wordlist.h"
#include "dice_entropy.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::string mnemonic_from_dice(
    std::string_view dice,
    std::size_t word_count
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
        cryptomachine::kBip39EnglishWordlist
    );
}

void test_12_word_pipeline() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    const std::string expected =
        "hole luggage safe present express tragic orbit shed "
        "switch metal identify path";

    const std::string actual =
        mnemonic_from_dice(
            dice,
            12
        );

    check(
        actual == expected,
        "12-word full C++ seed pipeline"
    );
}

void test_24_word_pipeline() {
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
            24
        );

    check(
        actual == expected,
        "24-word full C++ seed pipeline"
    );
}

}  // namespace

int main() {
    test_12_word_pipeline();
    test_24_word_pipeline();

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