#include "seed_engine.h"

#include "dice_entropy.h"
#include "dice_policy.h"

#include <cstdint>
#include <span>

namespace cryptomachine {

SeedEngineStatus create_seed_from_dice(
    std::string_view dice,
    std::size_t word_count,
    SeedResult& result
) {
    result = {};

    if (
        word_count != kWordCount12 &&
        word_count != kWordCount24
    ) {
        return SeedEngineStatus::UnsupportedWordCount;
    }

    if (!validate_outcome_count(
            dice,
            word_count
        )) {
        return SeedEngineStatus::InvalidDiceInput;
    }

    if (!analyze_five_dice(
            dice,
            result.sanity
        )) {
        result = {};
        return SeedEngineStatus::SanityAnalysisFailed;
    }

    DiceEntropy entropy;

    if (!dice_to_entropy(
            dice,
            word_count,
            entropy
        )) {
        result = {};
        return SeedEngineStatus::EntropyGenerationFailed;
    }

    const std::span<const std::uint8_t> entropy_span(
        entropy.bytes.data(),
        entropy.size
    );

    if (!bip39_entropy_to_mnemonic(
            entropy_span,
            result.mnemonic
        )) {
        result = {};
        return SeedEngineStatus::Bip39GenerationFailed;
    }

    return SeedEngineStatus::Success;
}

}  // namespace cryptomachine