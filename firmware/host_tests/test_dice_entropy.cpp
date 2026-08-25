#include "dice_entropy.h"

#include "sha256.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
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

std::string bytes_to_hex(
    const std::uint8_t* data,
    std::size_t size
) {
    std::ostringstream out;

    out << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < size; ++i) {
        out << std::setw(2)
            << static_cast<unsigned>(data[i]);
    }

    return out.str();
}

void test_12_word_vector() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    cryptomachine::DiceEntropy entropy;

    check(
        cryptomachine::dice_to_entropy(
            dice,
            12,
            entropy
        ),
        "12-word dice vector must convert successfully"
    );

    check(
        entropy.size == 16,
        "12-word dice entropy must be 16 bytes"
    );

    const auto digest = cryptomachine::sha256(dice);

    check(
        bytes_to_hex(entropy.bytes.data(), entropy.size) ==
        bytes_to_hex(digest.data(), 16),
        "12-word entropy must equal first 16 SHA-256 bytes"
    );
}

void test_24_word_vector() {
    constexpr std::string_view dice =
        "12345612345612345612345612345612345612345612345612"
        "34561234561234561234561234561234561234561234561234";

    cryptomachine::DiceEntropy entropy;

    check(
        cryptomachine::dice_to_entropy(
            dice,
            24,
            entropy
        ),
        "24-word dice vector must convert successfully"
    );

    check(
        entropy.size == 32,
        "24-word dice entropy must be 32 bytes"
    );

    const auto digest = cryptomachine::sha256(dice);

    check(
        bytes_to_hex(entropy.bytes.data(), entropy.size) ==
        bytes_to_hex(digest.data(), digest.size()),
        "24-word entropy must equal full SHA-256 digest"
    );
}

void test_invalid_inputs() {
    cryptomachine::DiceEntropy entropy;

    check(
        !cryptomachine::dice_to_entropy(
            "11111",
            12,
            entropy
        ),
        "short 12-word dice input must be rejected"
    );

    check(
        !cryptomachine::dice_to_entropy(
            std::string(50, '7'),
            12,
            entropy
        ),
        "invalid die values must be rejected"
    );

    check(
        !cryptomachine::dice_to_entropy(
            std::string(50, '1'),
            18,
            entropy
        ),
        "unsupported word count must be rejected"
    );
}

}  // namespace

int main() {
    test_12_word_vector();
    test_24_word_vector();
    test_invalid_inputs();

    if (failures != 0) {
        std::cerr
            << failures
            << " dice entropy test(s) failed.\n";

        return 1;
    }

    std::cout << "All C++ dice entropy tests passed.\n";
    return 0;
}