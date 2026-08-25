#include "dice_policy.h"

#include <stdexcept>

namespace cryptomachine {

std::size_t required_outcomes(
    const std::size_t word_count
) {
    if (word_count == kWordCount12) {
        return kOutcomes12;
    }

    if (word_count == kWordCount24) {
        return kOutcomes24;
    }

    throw std::invalid_argument(
        "word_count must be 12 or 24"
    );
}


std::size_t required_shakes(
    const std::size_t word_count
) {
    if (word_count == kWordCount12) {
        return kShakes12;
    }

    if (word_count == kWordCount24) {
        return kShakes24;
    }

    throw std::invalid_argument(
        "word_count must be 12 or 24"
    );
}


bool valid_dice_char(const char value) {
    return value >= '1' && value <= '6';
}


bool validate_outcome_count(
    const std::string_view dice,
    const std::size_t word_count
) {
    if (
        dice.size()
        != required_outcomes(word_count)
    ) {
        return false;
    }

    for (const char value : dice) {
        if (!valid_dice_char(value)) {
            return false;
        }
    }

    return true;
}

}  // namespace cryptomachine
