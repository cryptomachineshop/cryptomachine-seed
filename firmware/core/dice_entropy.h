#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace cryptomachine {

constexpr std::size_t kMaxEntropyBytes = 32;

struct DiceEntropy {
    std::array<std::uint8_t, kMaxEntropyBytes> bytes{};
    std::size_t size = 0;
};

// Converts a validated canonical dice string into BIP39 entropy.
//
// 12 words:
//   SHA-256(ASCII dice string), first 16 bytes.
//
// 24 words:
//   SHA-256(ASCII dice string), all 32 bytes.
//
// Returns false if the word count or dice input is invalid.
bool dice_to_entropy(
    std::string_view dice,
    std::size_t word_count,
    DiceEntropy& entropy
);

}  // namespace cryptomachine