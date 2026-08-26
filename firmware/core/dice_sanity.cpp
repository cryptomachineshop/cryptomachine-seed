#include "dice_sanity.h"
#include "secure_zero.h"

namespace cryptomachine {
namespace {

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

bool all_same(std::string_view sequence) {
    if (sequence.empty()) {
        return false;
    }

    const char first = sequence.front();

    for (const char face : sequence) {
        if (face != first) {
            return false;
        }
    }

    return true;
}

void add_aggregate_warning(
    AggregateDiceSanity& result,
    std::string warning
) {
    if (result.warning_count <
        result.warnings.size()) {
        result.warnings[result.warning_count] =
            std::move(warning);

        ++result.warning_count;
    }
}

void add_per_die_warning(
    PerDieSanity& result,
    std::string warning
) {
    if (result.warning_count <
        result.warnings.size()) {
        result.warnings[result.warning_count] =
            std::move(warning);

        ++result.warning_count;
    }
}

void add_combined_warning(
    FiveDiceSanity& result,
    const std::string& warning
) {
    if (result.warning_count <
        result.warnings.size()) {
        result.warnings[result.warning_count] =
            warning;

        ++result.warning_count;
    }
}

char most_common_face(
    std::string_view sequence,
    const DiceFaceCounts& counts
) {
    std::size_t highest_count = 0;

    for (const std::size_t count : counts) {
        if (count > highest_count) {
            highest_count = count;
        }
    }

    // Match Python Counter.most_common() tie behavior:
    // when counts tie, use the face encountered first
    // in the original sequence.
    for (const char face : sequence) {
        if (counts[face_index(face)] ==
            highest_count) {
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
    result = {};

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
        add_aggregate_warning(
            result,
            "One or more die faces never appeared "
            "in the combined results."
        );
    }

    if (result.longest_run >= 8) {
        add_aggregate_warning(
            result,
            "An unusually long repeated-face run "
            "was detected in the combined results."
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
            if (result.counts[i] * 3 >
                result.total) {
                const char face =
                    static_cast<char>('1' + i);

                add_aggregate_warning(
                    result,
                    std::string("Face ") +
                    face +
                    " appeared unusually often "
                    "in the combined results."
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
    result = {};

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
        std::size_t i = 0;
        i < result.aggregate.warning_count;
        ++i
    ) {
        add_combined_warning(
            result,
            result.aggregate.warnings[i]
        );
    }

    for (
        std::size_t die_index = 0;
        die_index < kDiceCount;
        ++die_index
    ) {
        PerDieSanity& die =
            result.per_die[die_index];

        die.rolls = result.rolls_per_die;

        die.sequence.reserve(
            result.rolls_per_die
        );

        for (
            std::size_t position = die_index;
            position < dice.size();
            position += kDiceCount
        ) {
            die.sequence.push_back(
                dice[position]
            );
        }

        for (const char face : die.sequence) {
            ++die.counts[face_index(face)];
        }

        die.longest_run =
            longest_identical_run(
                die.sequence
            );

        const std::string die_name =
            "D" +
            std::to_string(die_index + 1);

        const bool fixed_die =
            all_same(die.sequence);

        if (fixed_die) {
            add_per_die_warning(
                die,
                die_name +
                " produced the same face "
                "on every recorded shake."
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

        const char common_face =
            most_common_face(
                die.sequence,
                die.counts
            );

        const std::size_t common_count =
            die.counts[
                face_index(common_face)
            ];

        if (
            result.rolls_per_die >= 10 &&
            common_count >=
                concentration_threshold &&
            !fixed_die
        ) {
            add_per_die_warning(
                die,
                die_name +
                " produced face " +
                common_face +
                " on " +
                std::to_string(common_count) +
                " of " +
                std::to_string(
                    result.rolls_per_die
                ) +
                " shakes."
            );
        }

        if (
            die.longest_run >= 6 &&
            !fixed_die
        ) {
            add_per_die_warning(
                die,
                die_name +
                " produced the same face for " +
                std::to_string(
                    die.longest_run
                ) +
                " consecutive shakes."
            );
        }

        for (
            std::size_t i = 0;
            i < die.warning_count;
            ++i
        ) {
            add_combined_warning(
                result,
                die.warnings[i]
            );
        }
    }

    return true;
}

void destroy_five_dice_sanity(
    FiveDiceSanity& result
) {
    const auto wipe_string = [](std::string& value) {
        if (!value.empty()) {
            secure_zero(
                value.data(),
                value.size()
            );
        }

        value.clear();
    };

    secure_zero(
        result.aggregate.counts.data(),
        result.aggregate.counts.size() *
            sizeof(result.aggregate.counts[0])
    );

    secure_zero(
        result.aggregate.missing_faces.data(),
        result.aggregate.missing_faces.size() *
            sizeof(result.aggregate.missing_faces[0])
    );

    for (std::string& warning :
         result.aggregate.warnings) {
        wipe_string(warning);
    }

    for (PerDieSanity& die : result.per_die) {
        wipe_string(die.sequence);

        secure_zero(
            die.counts.data(),
            die.counts.size() *
                sizeof(die.counts[0])
        );

        for (std::string& warning :
             die.warnings) {
            wipe_string(warning);
        }
    }

    for (std::string& warning :
         result.warnings) {
        wipe_string(warning);
    }

    result = {};
}

}  // namespace cryptomachine