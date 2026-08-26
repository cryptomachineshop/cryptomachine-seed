#include "seed_engine.h"

#include "dice_entropy.h"
#include "dice_policy.h"
#include "secure_zero.h"

#include <cstdint>
#include <span>
#include <string>

namespace cryptomachine {
namespace {

void wipe_string(std::string& value) {
    if (!value.empty()) {
        secure_zero(
            value.data(),
            value.size()
        );
    }

    value.clear();
}

}  // namespace

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
        secure_zero(
            entropy.bytes.data(),
            entropy.bytes.size()
        );

        entropy.size = 0;

        result = {};

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
    // Wipe BIP39 word indexes.
    secure_zero(
        result.mnemonic.word_indices.data(),
        result.mnemonic.word_indices.size() *
            sizeof(result.mnemonic.word_indices[0])
    );

    result.mnemonic.word_count = 0;

    // Aggregate warnings.
    for (std::string& warning :
         result.sanity.aggregate.warnings) {
        wipe_string(warning);
    }

    // Per-die sequences and warnings.
    for (PerDieSanity& die :
         result.sanity.per_die) {
        wipe_string(die.sequence);

        for (std::string& warning :
             die.warnings) {
            wipe_string(warning);
        }
    }

    // Combined warning copies.
    for (std::string& warning :
         result.sanity.warnings) {
        wipe_string(warning);
    }

    // Once dynamically stored strings have been overwritten,
    // reset all remaining counters, counts, indexes, and metadata.
    result = {};
}

}  // namespace cryptomachine