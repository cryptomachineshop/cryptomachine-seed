#include <utility>
#include "dice_sanity.h"

#include <iostream>
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

bool has_warning(
    const cryptomachine::FiveDiceSanity& result,
    std::string_view expected
) {
    for (std::size_t i = 0; i < result.warning_count; ++i) {
        if (result.warnings[i] == expected) {
            return true;
        }
    }

    return false;
}

void test_balanced_aggregate() {
    std::string dice;

    for (int i = 0; i < 9; ++i) {
        dice += "123456";
    }

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(dice, result),
        "balanced aggregate input must be accepted"
    );

    check(result.total == 54, "balanced total must be 54");
    check(result.missing_face_count == 0, "balanced input has no missing faces");
    check(result.longest_run == 1, "balanced longest run must be 1");
    check(result.warning_count == 0, "balanced input must have no warnings");
}

void test_missing_face_warning() {
    std::string dice;

    for (int i = 0; i < 10; ++i) {
        dice += "12345";
    }

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(dice, result),
        "missing-face input must be accepted"
    );

    bool found_six = false;

    for (std::size_t i = 0; i < result.missing_face_count; ++i) {
        if (result.missing_faces[i] == '6') {
            found_six = true;
        }
    }

    check(found_six, "face 6 must be reported missing");
    check(result.warning_count != 0, "missing face must produce warning");
}

void test_long_run_warning() {
    std::string dice;

    for (int i = 0; i < 8; ++i) {
        dice += "123456";
    }

    dice += "111111111";

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(dice, result),
        "long-run input must be accepted"
    );

    check(result.longest_run >= 9, "long run must be detected");
    check(result.warning_count != 0, "long run must produce warning");
}

void test_gross_combined_bias_warning() {
    const std::string dice =
        std::string(40, '1') +
        "23456234562345623456";

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(dice, result),
        "biased aggregate input must be accepted"
    );

    check(
        result.counts[0] == 40,
        "face 1 aggregate count must be 40"
    );

    check(
        result.warning_count != 0,
        "gross aggregate bias must produce warning"
    );
}

void test_per_die_normal_pattern() {
    const std::string dice =
        "12345"
        "23456"
        "34561"
        "45612"
        "56123"
        "61234"
        "12345"
        "23456"
        "34561"
        "45612";

    cryptomachine::FiveDiceSanity result;

    check(
        cryptomachine::analyze_five_dice(dice, result),
        "normal five-dice input must be accepted"
    );

    check(
        result.rolls_per_die == 10,
        "normal pattern must contain 10 rolls per die"
    );

    check(
        result.warning_count == 0,
        "normal pattern must produce no warnings"
    );

    for (std::size_t i = 0; i < cryptomachine::kDiceCount; ++i) {
        check(
            result.per_die[i].rolls == 10,
            "each physical die must contain 10 rolls"
        );
    }
}

void test_stuck_die_detected() {
    const std::string dice =
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

    cryptomachine::FiveDiceSanity result;

    check(
        cryptomachine::analyze_five_dice(dice, result),
        "stuck-die input must be accepted"
    );

    check(
        result.per_die[0].counts[5] == 10,
        "D1 must contain ten face-6 results"
    );

    check(
        result.per_die[0].warning_count != 0,
        "stuck D1 must produce warning"
    );

    check(
        has_warning(
            result,
            "D1 produced the same face on every recorded shake."
        ),
        "combined warnings must report stuck D1"
    );
}

void test_heavy_single_die_bias_detected() {
    constexpr std::string_view d1 = "1111111123";
    constexpr std::string_view d2 = "1234561234";
    constexpr std::string_view d3 = "2345612345";
    constexpr std::string_view d4 = "3456123456";
    constexpr std::string_view d5 = "4561234561";

    std::string dice;

    for (std::size_t i = 0; i < 10; ++i) {
        dice.push_back(d1[i]);
        dice.push_back(d2[i]);
        dice.push_back(d3[i]);
        dice.push_back(d4[i]);
        dice.push_back(d5[i]);
    }

    cryptomachine::FiveDiceSanity result;

    check(
        cryptomachine::analyze_five_dice(dice, result),
        "heavy-bias input must be accepted"
    );

    check(
        result.per_die[0].counts[0] == 8,
        "D1 face-1 count must be 8"
    );

    check(
        has_warning(
            result,
            "D1 produced face 1 on 8 of 10 shakes."
        ),
        "heavy D1 concentration warning must match Python"
    );
}

void test_per_die_long_run_detected() {
    constexpr std::string_view d1 = "1111112345";
    constexpr std::string_view d2 = "1234561234";
    constexpr std::string_view d3 = "2345612345";
    constexpr std::string_view d4 = "3456123456";
    constexpr std::string_view d5 = "4561234561";

    std::string dice;

    for (std::size_t i = 0; i < 10; ++i) {
        dice.push_back(d1[i]);
        dice.push_back(d2[i]);
        dice.push_back(d3[i]);
        dice.push_back(d4[i]);
        dice.push_back(d5[i]);
    }

    cryptomachine::FiveDiceSanity result;

    check(
        cryptomachine::analyze_five_dice(dice, result),
        "per-die long-run input must be accepted"
    );

    check(
        result.per_die[0].longest_run == 6,
        "D1 longest run must equal 6"
    );

    check(
        has_warning(
            result,
            "D1 produced the same face for 6 consecutive shakes."
        ),
        "D1 long-run warning must match Python"
    );
}

void test_incomplete_shake_rejected() {
    cryptomachine::FiveDiceSanity result;

    check(
        !cryptomachine::analyze_five_dice(
            "123456",
            result
        ),
        "incomplete five-dice stream must be rejected"
    );
}

}  // namespace

int main() {
    test_balanced_aggregate();
    test_missing_face_warning();
    test_long_run_warning();
    test_gross_combined_bias_warning();
    test_per_die_normal_pattern();
    test_stuck_die_detected();
    test_heavy_single_die_bias_detected();
    test_per_die_long_run_detected();
    test_incomplete_shake_rejected();

    if (failures != 0) {
        std::cerr
            << failures
            << " dice sanity test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ dice sanity tests passed.\n";

    return 0;
}
