#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace cryptomachine {

using Sha256Digest = std::array<std::uint8_t, 32>;

Sha256Digest sha256(
    std::span<const std::uint8_t> data
);

Sha256Digest sha256(
    std::string_view text
);


}  // namespace cryptomachine
