#pragma once

#include "bip39.h"
#include "sha256.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace cryptomachine {

template <typename Wordlist>
inline std::string bip39_mnemonic_to_string(
    const Bip39Mnemonic& mnemonic,
    const Wordlist& wordlist
) {
    if (
        mnemonic.word_count != 12 &&
        mnemonic.word_count != 24
    ) {
        return {};
    }

    std::string result;

    for (
        std::size_t i = 0;
        i < mnemonic.word_count;
        ++i
    ) {
        const std::size_t index =
            mnemonic.word_indices[i];

        if (index >= wordlist.size()) {
            return {};
        }

        if (!result.empty()) {
            result.push_back(' ');
        }

        result.append(
            wordlist[index].data(),
            wordlist[index].size()
        );
    }

    return result;
}

inline std::string sha256_hex(
    const Sha256Digest& digest
) {
    static constexpr char kHex[] =
        "0123456789abcdef";

    std::string result;
    result.resize(digest.size() * 2);

    for (
        std::size_t index = 0;
        index < digest.size();
        ++index
    ) {
        const std::uint8_t value =
            digest[index];

        result[index * 2] =
            kHex[value >> 4U];

        result[index * 2 + 1] =
            kHex[value & 0x0FU];
    }

    return result;
}

}  // namespace cryptomachine
