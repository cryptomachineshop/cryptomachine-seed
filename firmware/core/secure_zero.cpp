#include "secure_zero.h"

namespace cryptomachine {

void secure_zero(
    void* data,
    std::size_t size
) {
    if (data == nullptr || size == 0) {
        return;
    }

    volatile std::uint8_t* bytes =
        static_cast<volatile std::uint8_t*>(data);

    for (std::size_t i = 0; i < size; ++i) {
        bytes[i] = 0;
    }
}

void secure_zero(
    std::span<std::uint8_t> data
) {
    secure_zero(
        data.data(),
        data.size()
    );
}

}  // namespace cryptomachine