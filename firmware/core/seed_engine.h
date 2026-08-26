#pragma once

#include "bip39.h"
#include "dice_sanity.h"

#include <cstddef>
#include <string_view>

namespace cryptomachine {

enum class SeedEngineStatus {
    Success = 0,
    UnsupportedWordCount,
    InvalidDiceInput,
    SanityAnalysisFailed,
    EntropyGenerationFailed,
    Bip39GenerationFailed,
};

struct SeedResult {
    Bip39Mnemonic mnemonic;
    FiveDiceSanity sanity;
};

// Production V1 dice-to-mnemonic entry point.
//
// This function:
//
// 1. Enforces the exact V1 dice outcome count.
// 2. Runs aggregate and per-die sanity analysis.
// 3. Generates deterministic BIP39 mnemonic indexes.
// 4. Returns the mnemonic plus sanity information.
//
// Sanity warnings never modify the cryptographic result.
SeedEngineStatus create_seed_from_dice(
    std::string_view dice,
    std::size_t word_count,
    SeedResult& result
);

}  // namespace cryptomachine