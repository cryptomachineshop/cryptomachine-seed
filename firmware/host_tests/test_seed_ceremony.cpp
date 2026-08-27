#include "test_format_helpers.h"
#include "bip39.h"
#include "bip39_wordlist.h"
#include "seed_ceremony.h"

#include <cstddef>
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

void test_12_word_ceremony() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    const std::string expected =
        "hole luggage safe present express tragic orbit shed "
        "switch metal identify path";

    cryptomachine::SeedCeremony ceremony;

    check(
        ceremony.initialize(12) ==
            cryptomachine::SeedCeremonyStatus::Success,
        "12-word ceremony must initialize"
    );

    check(
        ceremony.shake_count() == 0,
        "new ceremony must contain zero shakes"
    );

    check(
        ceremony.total_shakes() == 10,
        "12-word ceremony must require 10 shakes"
    );

    check(
        !ceremony.complete(),
        "new ceremony must not be complete"
    );

    check(
        !ceremony.generated(),
        "new ceremony must not be generated"
    );

    check(
        ceremony.result() == nullptr,
        "new ceremony must not expose a seed result"
    );

    check(
        ceremony.generate() ==
            cryptomachine::SeedCeremonyStatus::IncompleteDice,
        "incomplete ceremony must reject generation"
    );

    for (
        std::size_t offset = 0;
        offset < dice.size();
        offset += cryptomachine::kDiceCount
    ) {
        check(
            ceremony.add_shake(
                dice.substr(
                    offset,
                    cryptomachine::kDiceCount
                )
            ) ==
                cryptomachine::SeedCeremonyStatus::Success,
            "ceremony shake must be accepted"
        );
    }

    check(
        ceremony.shake_count() == 10,
        "completed ceremony must contain 10 shakes"
    );

    check(
        ceremony.remaining_shakes() == 0,
        "completed ceremony must have zero remaining shakes"
    );

    check(
        ceremony.complete(),
        "ceremony must report complete"
    );

    check(
        ceremony.generate() ==
            cryptomachine::SeedCeremonyStatus::Success,
        "completed ceremony must generate seed"
    );

    check(
        ceremony.generated(),
        "ceremony must report generated state"
    );

    const cryptomachine::SeedResult* result =
        ceremony.result();

    check(
        result != nullptr,
        "generated ceremony must expose result"
    );

    if (result != nullptr) {
        check(
            mnemonic_string(*result) == expected,
            "ceremony mnemonic must match known 12-word vector"
        );
    }

    check(
        ceremony.add_shake("12345") ==
            cryptomachine::SeedCeremonyStatus::AlreadyGenerated,
        "ceremony must reject dice after generation"
    );

    check(
        ceremony.undo_last_shake() ==
            cryptomachine::SeedCeremonyStatus::AlreadyGenerated,
        "ceremony must reject undo after generation"
    );

    check(
        ceremony.generate() ==
            cryptomachine::SeedCeremonyStatus::AlreadyGenerated,
        "ceremony must reject repeated generation"
    );
}

void test_destroy_session() {
    cryptomachine::SeedCeremony ceremony;

    check(
        ceremony.initialize(12) ==
            cryptomachine::SeedCeremonyStatus::Success,
        "destroy test ceremony must initialize"
    );

    for (std::size_t i = 0; i < 10; ++i) {
        check(
            ceremony.add_shake("12345") ==
                cryptomachine::SeedCeremonyStatus::Success,
            "destroy test shake must succeed"
        );
    }

    check(
        ceremony.generate() ==
            cryptomachine::SeedCeremonyStatus::Success,
        "destroy test generation must succeed"
    );

    check(
        ceremony.result() != nullptr,
        "destroy test must begin with seed result"
    );

    check(
        !ceremony.canonical_dice().empty(),
        "destroy test must begin with dice history"
    );

    ceremony.destroy_session();

    check(
        ceremony.shake_count() == 0,
        "destroy_session must clear shake count"
    );

    check(
        ceremony.canonical_dice().empty(),
        "destroy_session must clear dice history"
    );

    check(
        !ceremony.generated(),
        "destroy_session must clear generated state"
    );

    check(
        ceremony.result() == nullptr,
        "destroy_session must hide destroyed result"
    );

    check(
        ceremony.word_count() == 12,
        "destroy_session must preserve selected word count"
    );

    check(
        ceremony.total_shakes() == 10,
        "destroy_session must preserve ceremony mode"
    );

    check(
        ceremony.remaining_shakes() == 10,
        "destroy_session must restore full shake count"
    );

    check(
        ceremony.add_shake("54321") ==
            cryptomachine::SeedCeremonyStatus::Success,
        "destroyed ceremony must accept fresh dice"
    );
}

void test_undo_before_generation() {
    cryptomachine::SeedCeremony ceremony;

    check(
        ceremony.initialize(12) ==
            cryptomachine::SeedCeremonyStatus::Success,
        "undo ceremony must initialize"
    );

    check(
        ceremony.add_shake("62415") ==
            cryptomachine::SeedCeremonyStatus::Success,
        "undo test first shake must succeed"
    );

    check(
        ceremony.add_shake("35126") ==
            cryptomachine::SeedCeremonyStatus::Success,
        "undo test second shake must succeed"
    );

    check(
        ceremony.canonical_dice() == "6241535126",
        "undo test canonical dice must match"
    );

    check(
        ceremony.undo_last_shake() ==
            cryptomachine::SeedCeremonyStatus::Success,
        "undo before generation must succeed"
    );

    check(
        ceremony.canonical_dice() == "62415",
        "undo must remove final shake"
    );
}

void test_not_initialized() {
    cryptomachine::SeedCeremony ceremony;

    check(
        ceremony.add_shake("12345") ==
            cryptomachine::SeedCeremonyStatus::NotInitialized,
        "uninitialized ceremony must reject dice"
    );

    check(
        ceremony.undo_last_shake() ==
            cryptomachine::SeedCeremonyStatus::NotInitialized,
        "uninitialized ceremony must reject undo"
    );

    check(
        ceremony.generate() ==
            cryptomachine::SeedCeremonyStatus::NotInitialized,
        "uninitialized ceremony must reject generation"
    );
}

void test_invalid_initialization() {
    cryptomachine::SeedCeremony ceremony;

    check(
        ceremony.initialize(18) ==
            cryptomachine::SeedCeremonyStatus::UnsupportedWordCount,
        "unsupported ceremony word count must be rejected"
    );

    check(
        ceremony.word_count() == 0,
        "failed initialization must leave ceremony uninitialized"
    );

    check(
        ceremony.result() == nullptr,
        "failed initialization must expose no result"
    );
}

}  // namespace

int main() {
    test_12_word_ceremony();
    test_destroy_session();
    test_undo_before_generation();
    test_not_initialized();
    test_invalid_initialization();

    if (failures != 0) {
        std::cerr
            << failures
            << " seed ceremony test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ seed ceremony tests passed.\n";

    return 0;
}