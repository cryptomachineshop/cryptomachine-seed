#include "test_format_helpers.h"
#include "bip39.h"
#include "bip39_wordlist.h"
#include "seed_engine.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void check(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::string mnemonic_string(
    const cryptomachine::SeedResult& result
) {
    return cryptomachine::bip39_mnemonic_to_string(
        result.mnemonic,
        cryptomachine::kBip39EnglishWordlist
    );
}

void test_12_word_seed() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    const std::string expected =
        "hole luggage safe present express tragic orbit shed "
        "switch metal identify path";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            12,
            result
        );

    check(
        status == cryptomachine::SeedEngineStatus::Success,
        "12-word production seed must succeed"
    );

    check(
        mnemonic_string(result) == expected,
        "12-word production mnemonic must match Python"
    );

    check(
        result.mnemonic.word_count == 12,
        "12-word result must contain exactly 12 indexes"
    );
}

void test_24_word_seed() {
    constexpr std::string_view dice =
        "12345612345612345612345612345612345612345612345612"
        "34561234561234561234561234561234561234561234561234";

    const std::string expected =
        "tornado cactus wheel picture target finish home neither "
        "trend picture shoulder endless deputy glide open oxygen "
        "another ability forum swear side alcohol devote random";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            24,
            result
        );

    check(
        status == cryptomachine::SeedEngineStatus::Success,
        "24-word production seed must succeed"
    );

    check(
        mnemonic_string(result) == expected,
        "24-word production mnemonic must match Python"
    );

    check(
        result.mnemonic.word_count == 24,
        "24-word result must contain exactly 24 indexes"
    );
}

void test_warning_pattern_does_not_change_engine_contract() {
    // D1 is intentionally stuck on face 6. The product controller's
    // single pre-generation sanity pass warns about this pattern.
    // The seed engine itself is deliberately limited to validated
    // dice-to-mnemonic conversion and must remain deterministic.
    constexpr std::string_view dice =
        "61234"
        "62345"
        "63456"
        "64561"
        "65612"
        "66123"
        "61234"
        "62345"
        "63456"
        "64561";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            12,
            result
        );

    check(
        status == cryptomachine::SeedEngineStatus::Success,
        "valid dice values must remain generatable after sanity separation"
    );

    check(
        result.mnemonic.word_count == 12,
        "sanity separation must not alter mnemonic generation"
    );
}

void test_49_outcomes_rejected() {
    constexpr std::string_view dice =
        "6551522313165213216113315444412361646644311215344";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            12,
            result
        );

    check(
        status ==
            cryptomachine::SeedEngineStatus::InvalidDiceInput,
        "49-outcome production input must be rejected"
    );

    check(
        result.mnemonic.word_count == 0,
        "failed 49-outcome result must contain no mnemonic"
    );
}

void test_99_outcomes_rejected() {
    constexpr std::string_view dice =
        "12345612345612345612345612345612345612345612345612"
        "3456123456123456123456123456123456123456123456123";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            24,
            result
        );

    check(
        status ==
            cryptomachine::SeedEngineStatus::InvalidDiceInput,
        "99-outcome production input must be rejected"
    );

    check(
        result.mnemonic.word_count == 0,
        "failed 99-outcome result must contain no mnemonic"
    );
}

void test_unsupported_word_count_rejected() {
    const std::string dice(50, '1');

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            18,
            result
        );

    check(
        status ==
            cryptomachine::SeedEngineStatus::UnsupportedWordCount,
        "unsupported word count must return explicit status"
    );

    check(
        result.mnemonic.word_count == 0,
        "unsupported word count must leave result empty"
    );
}

void test_destroy_seed_result() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    cryptomachine::SeedResult result;

    const auto status =
        cryptomachine::create_seed_from_dice(
            dice,
            12,
            result
        );

    check(
        status == cryptomachine::SeedEngineStatus::Success,
        "destroy test seed generation must succeed"
    );

    check(
        result.mnemonic.word_count == 12,
        "destroy test must begin with mnemonic data"
    );

    cryptomachine::destroy_seed_result(result);

    check(
        result.mnemonic.word_count == 0,
        "destroy must clear mnemonic word count"
    );

    for (
        const std::uint16_t index :
        result.mnemonic.word_indices
    ) {
        check(
            index == 0,
            "destroy must clear every mnemonic index"
        );
    }
}

}  // namespace

int main() {
    test_12_word_seed();
    test_24_word_seed();
    test_warning_pattern_does_not_change_engine_contract();
    test_49_outcomes_rejected();
    test_99_outcomes_rejected();
    test_unsupported_word_count_rejected();
    test_destroy_seed_result();

    if (failures != 0) {
        std::cerr
            << failures
            << " seed engine test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ production seed engine tests passed.\n";

    return 0;
}
