#include "seed_engine.h"

#include "dice_entropy.h"
#include "dice_policy.h"
#include "secure_zero.h"

#include <cstdint>
#include <span>

namespace cryptomachine {

SeedEngineStatus create_seed_from_dice(
    std::string_view dice,
    std::size_t word_count,
    SeedResult& result
) {
    // Securely clear anything the caller may have left
    // in the result from an earlier generation.
    destroy_seed_result(result);

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
        destroy_seed_result(result);

        return SeedEngineStatus::SanityAnalysisFailed;
    }

    DiceEntropy entropy{};

    if (!dice_to_entropy(
            dice,
            word_count,
            entropy
        )) {
        secure_zero(
            entropy.bytes.data(),
            entropy.bytes.size()
        );

        entropy.size = 0;

        destroy_seed_result(result);

        return SeedEngineStatus::EntropyGenerationFailed;
    }

    const std::span<const std::uint8_t> entropy_span(
        entropy.bytes.data(),
        entropy.size
    );

    const bool bip39_ok =
        bip39_entropy_to_mnemonic(
            entropy_span,
            result.mnemonic
        );

    secure_zero(
        entropy.bytes.data(),
        entropy.bytes.size()
    );

    entropy.size = 0;

    if (!bip39_ok) {
        destroy_seed_result(result);

        return SeedEngineStatus::Bip39GenerationFailed;
    }

    return SeedEngineStatus::Success;
}

void destroy_seed_result(
    SeedResult& result
) {
    secure_zero(
        result.mnemonic.word_indices.data(),
        result.mnemonic.word_indices.size() *
            sizeof(result.mnemonic.word_indices[0])
    );

    result.mnemonic.word_count = 0;

    destroy_five_dice_sanity(
        result.sanity
    );

    result = {};
}

}  // namespace cryptomachine