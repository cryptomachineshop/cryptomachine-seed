#pragma once

#include "dice_policy.h"

#include <array>
#include <cstddef>
#include <cstdint>
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

enum class DiceSanityWarningCode : std::uint8_t {
    None = 0,
    AggregateMissingFaces,
    AggregateLongRun,
    AggregateFaceBias,
    PerDieFixed,
    PerDieFaceConcentration,
    PerDieLongRun,
};

// Fixed-size warning metadata.
//
// No warning text or dice sequence is stored here. The UI currently
// presents a generic warning, and these fields retain enough metadata
// for future deterministic formatting without retaining extra copies
// of the raw dice history.
struct DiceSanityWarning {
    DiceSanityWarningCode code =
        DiceSanityWarningCode::None;

    // 0 for aggregate warnings, 1 through 5 for per-die warnings.
    std::uint8_t die_number = 0;

    // Used by face-specific warnings, otherwise '\0'.
    char face = '\0';

    // Used by concentration/bias warnings.
    std::size_t observed = 0;
    std::size_t total = 0;

    // Used by repeated-run warnings.
    std::size_t run_length = 0;
};

struct AggregateDiceSanity {
    std::size_t total = 0;

    DiceFaceCounts counts{};

    std::array<char, kFaceCount> missing_faces{};
    std::size_t missing_face_count = 0;

    std::size_t longest_run = 0;

    std::array<
        DiceSanityWarning,
        kMaxAggregateWarnings
    > warnings{};

    std::size_t warning_count = 0;
};

struct PerDieSanity {
    std::size_t rolls = 0;

    DiceFaceCounts counts{};

    std::size_t longest_run = 0;

    std::array<
        DiceSanityWarning,
        kMaxPerDieWarnings
    > warnings{};

    std::size_t warning_count = 0;
};

struct FiveDiceSanity {
    AggregateDiceSanity aggregate;

    std::size_t rolls_per_die = 0;

    std::array<PerDieSanity, kDiceCount> per_die{};

    // Total warnings across aggregate + all five dice.
    // Warning records remain in their owning aggregate/per-die
    // structures instead of being copied into a second combined list.
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
// Analysis is allocation-free and does not construct per-die
// sequence copies. It scans the caller-owned canonical dice
// stream directly.
//
// Returns false for invalid input.
bool analyze_five_dice(
    std::string_view dice,
    FiveDiceSanity& result
);

// Securely wipes all fixed-size sanity metadata.
void destroy_five_dice_sanity(
    FiveDiceSanity& result
);

}  // namespace cryptomachine
