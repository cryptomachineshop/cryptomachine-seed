#include "bip39.h"

#include "secure_zero.h"
#include "sha256.h"

namespace cryptomachine {
namespace {

bool valid_entropy_size(std::size_t size) {
    return size == 16 ||
           size == 20 ||
           size == 24 ||
           size == 28 ||
           size == 32;
}

std::uint8_t entropy_bit(
    std::span<const std::uint8_t> entropy,
    std::size_t bit_index
) {
    const std::size_t byte_index = bit_index / 8;
    const std::size_t bit_in_byte = bit_index % 8;

    return static_cast<std::uint8_t>(
        (entropy[byte_index] >> (7 - bit_in_byte)) & 0x01U
    );
}

std::uint8_t checksum_bit(
    const Sha256Digest& hash,
    std::size_t bit_index
) {
    return static_cast<std::uint8_t>(
        (hash[0] >> (7 - bit_index)) & 0x01U
    );
}

}  // namespace

bool bip39_entropy_to_mnemonic(
    std::span<const std::uint8_t> entropy,
    Bip39Mnemonic& mnemonic
) {
    mnemonic = {};

    if (!valid_entropy_size(entropy.size())) {
        return false;
    }

    const std::size_t entropy_bits = entropy.size() * 8;
    const std::size_t checksum_bits = entropy_bits / 32;
    const std::size_t total_bits = entropy_bits + checksum_bits;
    const std::size_t word_count = total_bits / 11;

    Sha256Digest hash = sha256(entropy);

    for (std::size_t word = 0; word < word_count; ++word) {
        std::uint16_t index = 0;

        for (std::size_t bit = 0; bit < 11; ++bit) {
            const std::size_t global_bit = (word * 11) + bit;

            std::uint8_t value = 0;

            if (global_bit < entropy_bits) {
                value = entropy_bit(entropy, global_bit);
            } else {
                value = checksum_bit(
                    hash,
                    global_bit - entropy_bits
                );
            }

            index = static_cast<std::uint16_t>(
                (index << 1) | value
            );
        }

        mnemonic.word_indices[word] = index;
    }

    mnemonic.word_count = word_count;

    secure_zero(
        hash.data(),
        hash.size()
    );

    return true;
}


}  // namespace cryptomachine