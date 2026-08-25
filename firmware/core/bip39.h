#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace cryptomachine {

constexpr std::size_t kBip39WordlistSize = 2048;
constexpr std::size_t kBip39MaxWords = 24;

struct Bip39Mnemonic {
    std::array<std::uint16_t, kBip39MaxWords> word_indices{};
    std::size_t word_count = 0;
};

// Converts valid BIP39 entropy lengths into 0-based word indexes.
//
// Supported entropy sizes:
//   16 bytes -> 12 words
//   20 bytes -> 15 words
//   24 bytes -> 18 words
//   28 bytes -> 21 words
//   32 bytes -> 24 words
//
// Returns false for an invalid entropy size.
bool bip39_entropy_to_mnemonic(
    std::span<const std::uint8_t> entropy,
    Bip39Mnemonic& mnemonic
);

// Resolves a mnemonic's 0-based indexes against a 2048-word BIP39 list.
// Returns an empty string if the word list is not exactly 2048 entries
// or if the mnemonic structure contains an invalid index/count.
std::string bip39_mnemonic_to_string(
    const Bip39Mnemonic& mnemonic,
    std::span<const std::string_view> wordlist
);

}  // namespace cryptomachine