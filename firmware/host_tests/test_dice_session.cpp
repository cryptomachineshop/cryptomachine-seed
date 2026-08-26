#include "dice_session.h"

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

void test_12_word_session() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(12) ==
            cryptomachine::DiceSessionStatus::Success,
        "12-word session must initialize"
    );

    check(
        session.total_shakes() == 10,
        "12-word session must require 10 shakes"
    );

    check(
        !session.complete(),
        "new session must not be complete"
    );

    check(
        session.add_shake("62415") ==
            cryptomachine::DiceSessionStatus::Success,
        "first shake must be accepted"
    );

    check(
        session.add_shake("35126") ==
            cryptomachine::DiceSessionStatus::Success,
        "second shake must be accepted"
    );

    check(
        session.shake_count() == 2,
        "shake count must equal 2"
    );

    check(
        session.remaining_shakes() == 8,
        "remaining shakes must equal 8"
    );

    check(
        session.canonical_dice() == "6241535126",
        "canonical dice string must match Python"
    );

    check(
        session.undo_last_shake() ==
            cryptomachine::DiceSessionStatus::Success,
        "undo must succeed"
    );

    check(
        session.canonical_dice() == "62415",
        "canonical dice string after undo must match Python"
    );

    check(
        session.add_shake("35126") ==
            cryptomachine::DiceSessionStatus::Success,
        "re-added shake must succeed"
    );

    while (!session.complete()) {
        check(
            session.add_shake("12345") ==
                cryptomachine::DiceSessionStatus::Success,
            "remaining shakes must be accepted"
        );
    }

    check(
        session.shake_count() == 10,
        "completed 12-word session must contain 10 shakes"
    );

    check(
        session.canonical_dice().size() == 50,
        "completed 12-word session must contain 50 outcomes"
    );

    check(
        session.add_shake("12345") ==
            cryptomachine::DiceSessionStatus::SessionComplete,
        "completed session must reject another shake"
    );
}

void test_invalid_inputs() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(24) ==
            cryptomachine::DiceSessionStatus::Success,
        "24-word session must initialize"
    );

    check(
        session.add_shake("12347") ==
            cryptomachine::DiceSessionStatus::InvalidDiceValue,
        "invalid die value must be rejected"
    );

    check(
        session.add_shake("123") ==
            cryptomachine::DiceSessionStatus::InvalidShakeSize,
        "incomplete shake must be rejected"
    );

    check(
        session.shake_count() == 0,
        "invalid input must not advance shake count"
    );
}

void test_reset() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(24) ==
            cryptomachine::DiceSessionStatus::Success,
        "reset test session must initialize"
    );

    check(
        session.add_shake("66543") ==
            cryptomachine::DiceSessionStatus::Success,
        "reset test shake must be accepted"
    );

    check(
        !session.canonical_dice().empty(),
        "reset test must begin with stored dice"
    );

    session.reset();

    check(
        session.shake_count() == 0,
        "reset must clear shake count"
    );

    check(
        session.canonical_dice().empty(),
        "reset must clear canonical dice view"
    );

    check(
        session.word_count() == 24,
        "reset must preserve selected word count"
    );

    check(
        session.total_shakes() == 20,
        "reset must preserve total shake policy"
    );

    check(
        session.remaining_shakes() == 20,
        "reset must restore all remaining shakes"
    );
}

void test_undo_empty_session() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(12) ==
            cryptomachine::DiceSessionStatus::Success,
        "undo-empty session must initialize"
    );

    check(
        session.undo_last_shake() ==
            cryptomachine::DiceSessionStatus::NothingToUndo,
        "empty session undo must be rejected"
    );
}

void test_unsupported_word_count() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(18) ==
            cryptomachine::DiceSessionStatus::UnsupportedWordCount,
        "unsupported word count must be rejected"
    );

    check(
        session.word_count() == 0,
        "failed initialization must leave word count empty"
    );

    check(
        session.total_shakes() == 0,
        "failed initialization must leave total shakes empty"
    );

    check(
        session.canonical_dice().empty(),
        "failed initialization must contain no dice"
    );
}

void test_reinitialize_clears_previous_session() {
    cryptomachine::DiceSession session;

    check(
        session.initialize(12) ==
            cryptomachine::DiceSessionStatus::Success,
        "reinitialize test must start in 12-word mode"
    );

    check(
        session.add_shake("65432") ==
            cryptomachine::DiceSessionStatus::Success,
        "reinitialize test shake must succeed"
    );

    check(
        session.canonical_dice() == "65432",
        "reinitialize test must begin with stored dice"
    );

    check(
        session.initialize(24) ==
            cryptomachine::DiceSessionStatus::Success,
        "reinitialize into 24-word mode must succeed"
    );

    check(
        session.word_count() == 24,
        "reinitialize must select new word count"
    );

    check(
        session.total_shakes() == 20,
        "reinitialize must select new shake policy"
    );

    check(
        session.shake_count() == 0,
        "reinitialize must clear previous shake count"
    );

    check(
        session.canonical_dice().empty(),
        "reinitialize must clear previous dice"
    );
}

}  // namespace

int main() {
    test_12_word_session();
    test_invalid_inputs();
    test_reset();
    test_undo_empty_session();
    test_unsupported_word_count();
    test_reinitialize_clears_previous_session();

    if (failures != 0) {
        std::cerr
            << failures
            << " dice session test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ dice session tests passed.\n";

    return 0;
}