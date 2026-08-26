#pragma once

#include "dice_policy.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace cryptomachine {

constexpr std::size_t kFaceCount = 6;

constexpr std::size_t kMaxAggregateWarnings = 3;
constexpr std::size_t kMaxPerDieWarnings = 2;
constexpr std::size_t kMaxFiveDiceWarnings =
    kMaxAggregateWarnings +
    (kDiceCount * kMaxPerDieWarnings);

using DiceFaceCounts =
    std::array<std::size_t, kFaceCount>;

struct AggregateDiceSanity {
    std::size_t total = 0;

    DiceFaceCounts counts{};

    std::array<char, kFaceCount> missing_faces{};
    std::size_t missing_face_count = 0;

    std::size_t longest_run = 0;

    std::array<std::string, kMaxAggregateWarnings>
        warnings{};

    std::size_t warning_count = 0;
};

struct PerDieSanity {
    std::size_t rolls = 0;

    std::string sequence;

    DiceFaceCounts counts{};

    std::size_t longest_run = 0;

    std::array<std::string, kMaxPerDieWarnings>
        warnings{};

    std::size_t warning_count = 0;
};

struct FiveDiceSanity {
    AggregateDiceSanity aggregate;

    std::size_t rolls_per_die = 0;

    std::array<PerDieSanity, kDiceCount> per_die{};

    std::array<std::string, kMaxFiveDiceWarnings>
        warnings{};

    std::size_t warning_count = 0;
};

// Aggregate analysis equivalent to Python analyze_dice().
//
// Returns false for empty input or any character
// outside ASCII digits '1' through '6'.
bool analyze_dice(
    std::string_view dice,
    AggregateDiceSanity& result
);

// Fixed-position D1-D5 analysis equivalent to
// Python analyze_five_dice().
//
// Input must contain a complete multiple of five
// valid dice outcomes.
//
// Returns false for invalid input.
bool analyze_five_dice(
    std::string_view dice,
    FiveDiceSanity& result
);

void destroy_five_dice_sanity(
    FiveDiceSanity& result
);

}  // namespace cryptomachine