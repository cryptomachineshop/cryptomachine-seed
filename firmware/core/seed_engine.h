#pragma once

#include "bip39.h"

#include <cstddef>
#include <string_view>

namespace cryptomachine {

enum class SeedEngineStatus {
    Success = 0,
    UnsupportedWordCount,
    InvalidDiceInput,
    EntropyGenerationFailed,
    Bip39GenerationFailed,
};

struct SeedResult {
    Bip39Mnemonic mnemonic;
};

// Production V1 dice-to-mnemonic entry point.
//
// This function:
//
// 1. Enforces the exact V1 dice outcome count.
// 2. Generates deterministic entropy from the canonical dice string.
// 3. Converts that entropy to BIP39 mnemonic indexes.
//
// Dice sanity analysis intentionally lives outside the seed engine.
// The product controller performs the single pre-generation sanity
// pass and owns the resulting report for the lifetime of the session.
SeedEngineStatus create_seed_from_dice(
    std::string_view dice,
    std::size_t word_count,
    SeedResult& result
);

// Explicitly wipe sensitive data held by a completed or
// partially completed seed result.
//
// Intended for success, cancel, timeout, reset, and session
// teardown paths.
void destroy_seed_result(
    SeedResult& result
);

}  // namespace cryptomachine
