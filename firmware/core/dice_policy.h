#pragma once

#include <cstddef>
#include <string_view>

namespace cryptomachine {

constexpr std::size_t kDiceCount = 5;

constexpr std::size_t kWordCount12 = 12;
constexpr std::size_t kWordCount24 = 24;

constexpr std::size_t kOutcomes12 = 50;
constexpr std::size_t kOutcomes24 = 100;

constexpr std::size_t kShakes12 = 10;
constexpr std::size_t kShakes24 = 20;

std::size_t required_outcomes(
    std::size_t word_count
);

std::size_t required_shakes(
    std::size_t word_count
);

bool valid_dice_char(char value);

bool validate_outcome_count(
    std::string_view dice,
    std::size_t word_count
);

}  // namespace cryptomachine
