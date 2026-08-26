#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace cryptomachine {

// Overwrite a memory region using volatile writes so the compiler
// cannot remove the clearing operation as a dead store.
void secure_zero(
    void* data,
    std::size_t size
);

// Convenience overload for mutable byte spans.
void secure_zero(
    std::span<std::uint8_t> data
);

}  // namespace cryptomachine