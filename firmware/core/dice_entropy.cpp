#include "dice_entropy.h"

#include "dice_policy.h"
#include "sha256.h"

namespace cryptomachine {

bool dice_to_entropy(
    std::string_view dice,
    std::size_t word_count,
    DiceEntropy& entropy
) {
    entropy = {};

    if (word_count != kWordCount12 &&
        word_count != kWordCount24) {
        return false;
    }

    if (!validate_outcome_count(dice, word_count)) {
        return false;
    }

    const Sha256Digest digest = sha256(dice);

    if (word_count == kWordCount12) {
        for (std::size_t i = 0; i < 16; ++i) {
            entropy.bytes[i] = digest[i];
        }

        entropy.size = 16;
        return true;
    }

    for (std::size_t i = 0; i < digest.size(); ++i) {
        entropy.bytes[i] = digest[i];
    }

    entropy.size = digest.size();
    return true;
}

}  // namespace cryptomachine