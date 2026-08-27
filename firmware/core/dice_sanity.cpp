#include "dice_sanity.h"

#include "secure_zero.h"

#include <type_traits>

namespace cryptomachine {
namespace {

static_assert(
    std::is_trivially_copyable_v<AggregateDiceSanity>
);

static_assert(
    std::is_trivially_copyable_v<PerDieSanity>
);

static_assert(
    std::is_trivially_copyable_v<FiveDiceSanity>
);

bool valid_dice_string(std::string_view dice) {
    if (dice.empty()) {
        return false;
    }

    for (const char face : dice) {
        if (!valid_dice_char(face)) {
            return false;
        }
    }

    return true;
}

std::size_t face_index(const char face) {
    return static_cast<std::size_t>(face - '1');
}

std::size_t longest_identical_run(
    std::string_view sequence
) {
    if (sequence.empty()) {
        return 0;
    }

    std::size_t longest = 1;
    std::size_t current = 1;

    for (std::size_t i = 1; i < sequence.size(); ++i) {
        if (sequence[i] == sequence[i - 1]) {
            ++current;

            if (current > longest) {
                longest = current;
            }
        } else {
            current = 1;
        }
    }

    return longest;
}

void clear_aggregate(
    AggregateDiceSanity& result
) {
    secure_zero(
        &result,
        sizeof(result)
    );
}

void add_aggregate_warning(
    AggregateDiceSanity& result,
    const DiceSanityWarning& warning
) {
    if (
        result.warning_count <
        result.warnings.size()
    ) {
        result.warnings[result.warning_count] =
            warning;

        ++result.warning_count;
    }
}

void add_per_die_warning(
    PerDieSanity& result,
    const DiceSanityWarning& warning
) {
    if (
        result.warning_count <
        result.warnings.size()
    ) {
        result.warnings[result.warning_count] =
            warning;

        ++result.warning_count;
    }
}

char most_common_face_for_die(
    std::string_view dice,
    std::size_t die_index,
    const DiceFaceCounts& counts
) {
    std::size_t highest_count = 0;

    for (const std::size_t count : counts) {
        if (count > highest_count) {
            highest_count = count;
        }
    }

    // Match Python Counter.most_common() tie behavior without
    // constructing a per-die sequence: among equally common
    // faces, return whichever was encountered first for this
    // physical die in the canonical interleaved dice stream.
    for (
        std::size_t position = die_index;
        position < dice.size();
        position += kDiceCount
    ) {
        const char face = dice[position];

        if (
            counts[face_index(face)] ==
            highest_count
        ) {
            return face;
        }
    }

    return '1';
}

}  // namespace

bool analyze_dice(
    std::string_view dice,
    AggregateDiceSanity& result
) {
    // Safe on reuse because the result is fixed-size and
    // trivially copyable. No heap-backed strings are released.
    clear_aggregate(result);

    if (!valid_dice_string(dice)) {
        return false;
    }

    result.total = dice.size();

    for (const char face : dice) {
        ++result.counts[face_index(face)];
    }

    for (std::size_t i = 0; i < kFaceCount; ++i) {
        if (result.counts[i] == 0) {
            result.missing_faces[
                result.missing_face_count
            ] = static_cast<char>('1' + i);

            ++result.missing_face_count;
        }
    }

    result.longest_run =
        longest_identical_run(dice);

    if (result.missing_face_count != 0) {
        DiceSanityWarning warning{};
        warning.code =
            DiceSanityWarningCode::
                AggregateMissingFaces;

        add_aggregate_warning(
            result,
            warning
        );
    }

    if (result.longest_run >= 8) {
        DiceSanityWarning warning{};
        warning.code =
            DiceSanityWarningCode::
                AggregateLongRun;
        warning.run_length =
            result.longest_run;

        add_aggregate_warning(
            result,
            warning
        );
    }

    if (result.total >= 50) {
        // Python rule:
        //
        // expected = total / 6
        // count > expected * 2
        //
        // Equivalent integer comparison:
        //
        // count * 3 > total
        for (std::size_t i = 0; i < kFaceCount; ++i) {
            if (
                result.counts[i] * 3 >
                result.total
            ) {
                DiceSanityWarning warning{};
                warning.code =
                    DiceSanityWarningCode::
                        AggregateFaceBias;
                warning.face =
                    static_cast<char>('1' + i);
                warning.observed =
                    result.counts[i];
                warning.total =
                    result.total;

                add_aggregate_warning(
                    result,
                    warning
                );

                break;
            }
        }
    }

    return true;
}

bool analyze_five_dice(
    std::string_view dice,
    FiveDiceSanity& result
) {
    // Full-object wipe is valid because the report contains
    // only fixed-size trivially-copyable data.
    secure_zero(
        &result,
        sizeof(result)
    );

    if (!valid_dice_string(dice)) {
        return false;
    }

    if (dice.size() % kDiceCount != 0) {
        return false;
    }

    if (!analyze_dice(
            dice,
            result.aggregate
        )) {
        return false;
    }

    result.rolls_per_die =
        dice.size() / kDiceCount;

    for (
        std::size_t die_index = 0;
        die_index < kDiceCount;
        ++die_index
    ) {
        PerDieSanity& die =
            result.per_die[die_index];

        die.rolls =
            result.rolls_per_die;

        char previous_face = '\0';
        std::size_t current_run = 0;

        for (
            std::size_t position = die_index;
            position < dice.size();
            position += kDiceCount
        ) {
            const char face =
                dice[position];

            ++die.counts[
                face_index(face)
            ];

            if (
                current_run == 0 ||
                face != previous_face
            ) {
                current_run = 1;
            } else {
                ++current_run;
            }

            if (
                current_run >
                die.longest_run
            ) {
                die.longest_run =
                    current_run;
            }

            previous_face = face;
        }

        const char common_face =
            most_common_face_for_die(
                dice,
                die_index,
                die.counts
            );

        const std::size_t common_count =
            die.counts[
                face_index(common_face)
            ];

        const bool fixed_die =
            common_count ==
            result.rolls_per_die;

        if (fixed_die) {
            DiceSanityWarning warning{};
            warning.code =
                DiceSanityWarningCode::
                    PerDieFixed;
            warning.die_number =
                static_cast<std::uint8_t>(
                    die_index + 1
                );
            warning.face =
                common_face;
            warning.observed =
                common_count;
            warning.total =
                result.rolls_per_die;

            add_per_die_warning(
                die,
                warning
            );
        }

        // Python:
        //
        // ceil(rolls_per_die * 0.75)
        //
        // Exact integer equivalent:
        //
        // ceil(3 * rolls / 4)
        const std::size_t
            concentration_threshold =
                (
                    (result.rolls_per_die * 3)
                    + 3
                ) / 4;

        if (
            result.rolls_per_die >= 10 &&
            common_count >=
                concentration_threshold &&
            !fixed_die
        ) {
            DiceSanityWarning warning{};
            warning.code =
                DiceSanityWarningCode::
                    PerDieFaceConcentration;
            warning.die_number =
                static_cast<std::uint8_t>(
                    die_index + 1
                );
            warning.face =
                common_face;
            warning.observed =
                common_count;
            warning.total =
                result.rolls_per_die;

            add_per_die_warning(
                die,
                warning
            );
        }

        if (
            die.longest_run >= 6 &&
            !fixed_die
        ) {
            DiceSanityWarning warning{};
            warning.code =
                DiceSanityWarningCode::
                    PerDieLongRun;
            warning.die_number =
                static_cast<std::uint8_t>(
                    die_index + 1
                );
            warning.run_length =
                die.longest_run;

            add_per_die_warning(
                die,
                warning
            );
        }
    }

    result.warning_count =
        result.aggregate.warning_count;

    for (const PerDieSanity& die : result.per_die) {
        result.warning_count +=
            die.warning_count;
    }

    return true;
}

void destroy_five_dice_sanity(
    FiveDiceSanity& result
) {
    secure_zero(
        &result,
        sizeof(result)
    );
}

}  // namespace cryptomachine
