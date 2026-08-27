#include "dice_sanity.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

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

const cryptomachine::DiceSanityWarning*
find_aggregate_warning(
    const cryptomachine::AggregateDiceSanity& result,
    cryptomachine::DiceSanityWarningCode code
) {
    for (
        std::size_t i = 0;
        i < result.warning_count;
        ++i
    ) {
        if (result.warnings[i].code == code) {
            return &result.warnings[i];
        }
    }

    return nullptr;
}

const cryptomachine::DiceSanityWarning*
find_per_die_warning(
    const cryptomachine::PerDieSanity& result,
    cryptomachine::DiceSanityWarningCode code
) {
    for (
        std::size_t i = 0;
        i < result.warning_count;
        ++i
    ) {
        if (result.warnings[i].code == code) {
            return &result.warnings[i];
        }
    }

    return nullptr;
}

void test_report_is_fixed_size() {
    static_assert(
        std::is_trivially_copyable_v<
            cryptomachine::FiveDiceSanity
        >
    );

    static_assert(
        std::is_trivially_copyable_v<
            cryptomachine::DiceSanityWarning
        >
    );

    check(
        true,
        "fixed-size sanity report must compile"
    );
}

void test_balanced_aggregate() {
    std::string dice;

    for (int i = 0; i < 9; ++i) {
        dice += "123456";
    }

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(
            dice,
            result
        ),
        "balanced aggregate input must be accepted"
    );

    check(
        result.total == 54,
        "balanced total must be 54"
    );

    check(
        result.missing_face_count == 0,
        "balanced input has no missing faces"
    );

    check(
        result.longest_run == 1,
        "balanced longest run must be 1"
    );

    check(
        result.warning_count == 0,
        "balanced input must have no warnings"
    );
}

void test_missing_face_warning() {
    std::string dice;

    for (int i = 0; i < 10; ++i) {
        dice += "12345";
    }

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(
            dice,
            result
        ),
        "missing-face input must be accepted"
    );

    bool found_six = false;

    for (
        std::size_t i = 0;
        i < result.missing_face_count;
        ++i
    ) {
        if (result.missing_faces[i] == '6') {
            found_six = true;
        }
    }

    check(
        found_six,
        "face 6 must be reported missing"
    );

    check(
        find_aggregate_warning(
            result,
            cryptomachine::DiceSanityWarningCode::
                AggregateMissingFaces
        ) != nullptr,
        "missing face must produce coded warning"
    );
}

void test_long_run_warning() {
    std::string dice;

    for (int i = 0; i < 8; ++i) {
        dice += "123456";
    }

    dice += "111111111";

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(
            dice,
            result
        ),
        "long-run input must be accepted"
    );

    check(
        result.longest_run >= 9,
        "long run must be detected"
    );

    const auto* warning =
        find_aggregate_warning(
            result,
            cryptomachine::DiceSanityWarningCode::
                AggregateLongRun
        );

    check(
        warning != nullptr,
        "long run must produce coded warning"
    );

    if (warning != nullptr) {
        check(
            warning->run_length >= 9,
            "long-run warning must preserve run length"
        );
    }
}

void test_gross_combined_bias_warning() {
    const std::string dice =
        std::string(40, '1') +
        "23456234562345623456";

    cryptomachine::AggregateDiceSanity result;

    check(
        cryptomachine::analyze_dice(
            dice,
            result
        ),
        "biased aggregate input must be accepted"
    );

    check(
        result.counts[0] == 40,
        "face 1 aggregate count must be 40"
    );

    const auto* warning =
        find_aggregate_warning(
            result,
            cryptomachine::DiceSanityWarningCode::
                AggregateFaceBias
        );

    check(
        warning != nullptr,
        "gross aggregate bias must produce coded warning"
    );

    if (warning != nullptr) {
        check(
            warning->face == '1',
            "aggregate bias warning must preserve face"
        );

        check(
            warning->observed == 40,
            "aggregate bias warning must preserve count"
        );

        check(
            warning->total == dice.size(),
            "aggregate bias warning must preserve total"
        );
    }
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
        cryptomachine::analyze_five_dice(
            dice,
            result
        ),
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

    for (
        std::size_t i = 0;
        i < cryptomachine::kDiceCount;
        ++i
    ) {
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
        cryptomachine::analyze_five_dice(
            dice,
            result
        ),
        "stuck-die input must be accepted"
    );

    check(
        result.per_die[0].counts[5] == 10,
        "D1 must contain ten face-6 results"
    );

    const auto* warning =
        find_per_die_warning(
            result.per_die[0],
            cryptomachine::DiceSanityWarningCode::
                PerDieFixed
        );

    check(
        warning != nullptr,
        "stuck D1 must produce fixed-die warning"
    );

    if (warning != nullptr) {
        check(
            warning->die_number == 1,
            "stuck warning must identify D1"
        );

        check(
            warning->face == '6',
            "stuck warning must preserve face 6"
        );

        check(
            warning->observed == 10 &&
            warning->total == 10,
            "stuck warning must preserve 10 of 10"
        );
    }

    check(
        result.warning_count != 0,
        "stuck D1 must contribute to total warning count"
    );
}

void test_heavy_single_die_bias_detected() {
    constexpr std::string_view d1 =
        "1111111123";
    constexpr std::string_view d2 =
        "1234561234";
    constexpr std::string_view d3 =
        "2345612345";
    constexpr std::string_view d4 =
        "3456123456";
    constexpr std::string_view d5 =
        "4561234561";

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
        cryptomachine::analyze_five_dice(
            dice,
            result
        ),
        "heavy-bias input must be accepted"
    );

    check(
        result.per_die[0].counts[0] == 8,
        "D1 face-1 count must be 8"
    );

    const auto* warning =
        find_per_die_warning(
            result.per_die[0],
            cryptomachine::DiceSanityWarningCode::
                PerDieFaceConcentration
        );

    check(
        warning != nullptr,
        "heavy D1 concentration must produce coded warning"
    );

    if (warning != nullptr) {
        check(
            warning->die_number == 1,
            "concentration warning must identify D1"
        );

        check(
            warning->face == '1',
            "concentration warning must preserve face 1"
        );

        check(
            warning->observed == 8 &&
            warning->total == 10,
            "concentration warning must preserve 8 of 10"
        );
    }
}

void test_per_die_long_run_detected() {
    constexpr std::string_view d1 =
        "1111112345";
    constexpr std::string_view d2 =
        "1234561234";
    constexpr std::string_view d3 =
        "2345612345";
    constexpr std::string_view d4 =
        "3456123456";
    constexpr std::string_view d5 =
        "4561234561";

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
        cryptomachine::analyze_five_dice(
            dice,
            result
        ),
        "per-die long-run input must be accepted"
    );

    check(
        result.per_die[0].longest_run == 6,
        "D1 longest run must equal 6"
    );

    const auto* warning =
        find_per_die_warning(
            result.per_die[0],
            cryptomachine::DiceSanityWarningCode::
                PerDieLongRun
        );

    check(
        warning != nullptr,
        "D1 long run must produce coded warning"
    );

    if (warning != nullptr) {
        check(
            warning->die_number == 1,
            "long-run warning must identify D1"
        );

        check(
            warning->run_length == 6,
            "long-run warning must preserve run length 6"
        );
    }
}

void test_destroy_five_dice_sanity() {
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
        cryptomachine::analyze_five_dice(
            dice,
            result
        ),
        "destruction test input must be accepted"
    );

    check(
        result.rolls_per_die != 0,
        "destruction test must begin with roll data"
    );

    check(
        result.warning_count != 0,
        "destruction test must begin with warnings"
    );

    cryptomachine::destroy_five_dice_sanity(
        result
    );

    check(
        result.aggregate.total == 0,
        "destruction must clear aggregate total"
    );

    check(
        result.aggregate.missing_face_count == 0,
        "destruction must clear missing-face count"
    );

    check(
        result.aggregate.longest_run == 0,
        "destruction must clear aggregate longest run"
    );

    check(
        result.aggregate.warning_count == 0,
        "destruction must clear aggregate warnings"
    );

    for (
        std::size_t count :
        result.aggregate.counts
    ) {
        check(
            count == 0,
            "destruction must clear aggregate counts"
        );
    }

    for (
        const auto& warning :
        result.aggregate.warnings
    ) {
        check(
            warning.code ==
                cryptomachine::DiceSanityWarningCode::None,
            "destruction must clear aggregate warning codes"
        );
    }

    for (const auto& die : result.per_die) {
        check(
            die.rolls == 0,
            "destruction must clear per-die roll count"
        );

        check(
            die.longest_run == 0,
            "destruction must clear per-die longest run"
        );

        check(
            die.warning_count == 0,
            "destruction must clear per-die warnings"
        );

        for (std::size_t count : die.counts) {
            check(
                count == 0,
                "destruction must clear per-die counts"
            );
        }

        for (const auto& warning : die.warnings) {
            check(
                warning.code ==
                    cryptomachine::DiceSanityWarningCode::None,
                "destruction must clear per-die warning codes"
            );
        }
    }

    check(
        result.rolls_per_die == 0,
        "destruction must clear rolls-per-die"
    );

    check(
        result.warning_count == 0,
        "destruction must clear total warning count"
    );
}

void test_reuse_clears_previous_report() {
    const std::string warning_dice =
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

    const std::string clean_dice =
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
        cryptomachine::analyze_five_dice(
            warning_dice,
            result
        ),
        "reuse setup warning input must be accepted"
    );

    check(
        result.warning_count != 0,
        "reuse setup must begin with warnings"
    );

    check(
        cryptomachine::analyze_five_dice(
            clean_dice,
            result
        ),
        "reuse clean input must be accepted"
    );

    check(
        result.warning_count == 0,
        "reuse must securely replace prior warning metadata"
    );

    check(
        result.per_die[0].warning_count == 0,
        "reuse must clear prior per-die warnings"
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

    check(
        result.warning_count == 0,
        "invalid input must leave cleared warning state"
    );
}

}  // namespace

int main() {
    test_report_is_fixed_size();
    test_balanced_aggregate();
    test_missing_face_warning();
    test_long_run_warning();
    test_gross_combined_bias_warning();
    test_per_die_normal_pattern();
    test_stuck_die_detected();
    test_heavy_single_die_bias_detected();
    test_per_die_long_run_detected();
    test_destroy_five_dice_sanity();
    test_reuse_clears_previous_report();
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
