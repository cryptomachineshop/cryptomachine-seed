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

bool has_warning_containing(
    const cryptomachine::FiveDiceSanity& sanity,
    std::string_view text
) {
    for (
        std::size_t i = 0;
        i < sanity.warning_count;
        ++i
    ) {
        if (
            sanity.warnings[i].find(text)
            != std::string::npos
        ) {
            return true;
        }
    }

    return false;
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
        result.sanity.rolls_per_die == 10,
        "12-word sanity must contain 10 rolls per die"
    );

    for (
        std::size_t i = 0;
        i < cryptomachine::kDiceCount;
        ++i
    ) {
        check(
            result.sanity.per_die[i].rolls == 10,
            "each physical die must contain 10 rolls"
        );
    }
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
        result.sanity.rolls_per_die == 20,
        "24-word sanity must contain 20 rolls per die"
    );
}

void test_per_die_warning_reaches_seed_engine() {
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
        "sanity warning must not block seed generation"
    );

    check(
        has_warning_containing(
            result.sanity,
            "D1 produced the same face"
        ),
        "D1 sanity warning must reach seed engine result"
    );

    check(
        result.mnemonic.word_count == 12,
        "sanity warning must not alter mnemonic generation"
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
        "destroy test seed generation must succeed"
    );

    check(
        result.mnemonic.word_count == 12,
        "destroy test must begin with mnemonic data"
    );

    check(
        !result.sanity.per_die[0].sequence.empty(),
        "destroy test must begin with stored dice sequence"
    );

    check(
        result.sanity.warning_count != 0,
        "destroy test must begin with sanity warning data"
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

    check(
        result.sanity.rolls_per_die == 0,
        "destroy must clear rolls-per-die metadata"
    );

    check(
        result.sanity.warning_count == 0,
        "destroy must clear combined warning count"
    );

    check(
        result.sanity.aggregate.total == 0,
        "destroy must clear aggregate total"
    );

    for (
        const auto& die :
        result.sanity.per_die
    ) {
        check(
            die.sequence.empty(),
            "destroy must clear per-die sequence"
        );

        check(
            die.warning_count == 0,
            "destroy must clear per-die warning count"
        );
    }
}

}  // namespace

int main() {
    test_12_word_seed();
    test_24_word_seed();
    test_per_die_warning_reaches_seed_engine();
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