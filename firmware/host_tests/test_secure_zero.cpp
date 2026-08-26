#include "secure_zero.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_pointer_wipe() {
    std::array<std::uint8_t, 32> buffer{};

    buffer.fill(0xa5);

    cryptomachine::secure_zero(
        buffer.data(),
        buffer.size()
    );

    for (const std::uint8_t value : buffer) {
        check(
            value == 0,
            "pointer-based secure_zero must clear every byte"
        );
    }
}

void test_span_wipe() {
    std::array<std::uint8_t, 16> buffer{};

    buffer.fill(0xff);

    cryptomachine::secure_zero(
        std::span<std::uint8_t>(
            buffer.data(),
            buffer.size()
        )
    );

    for (const std::uint8_t value : buffer) {
        check(
            value == 0,
            "span-based secure_zero must clear every byte"
        );
    }
}

void test_partial_wipe() {
    std::array<std::uint8_t, 8> buffer{
        1, 2, 3, 4, 5, 6, 7, 8
    };

    cryptomachine::secure_zero(
        buffer.data() + 2,
        4
    );

    check(buffer[0] == 1, "byte 0 must remain unchanged");
    check(buffer[1] == 2, "byte 1 must remain unchanged");

    check(buffer[2] == 0, "byte 2 must be cleared");
    check(buffer[3] == 0, "byte 3 must be cleared");
    check(buffer[4] == 0, "byte 4 must be cleared");
    check(buffer[5] == 0, "byte 5 must be cleared");

    check(buffer[6] == 7, "byte 6 must remain unchanged");
    check(buffer[7] == 8, "byte 7 must remain unchanged");
}

void test_null_and_zero_size() {
    cryptomachine::secure_zero(
        nullptr,
        0
    );

    std::array<std::uint8_t, 4> buffer{
        9, 8, 7, 6
    };

    cryptomachine::secure_zero(
        buffer.data(),
        0
    );

    check(buffer[0] == 9, "zero-size wipe must not modify buffer");
    check(buffer[1] == 8, "zero-size wipe must not modify buffer");
    check(buffer[2] == 7, "zero-size wipe must not modify buffer");
    check(buffer[3] == 6, "zero-size wipe must not modify buffer");
}

}  // namespace

int main() {
    test_pointer_wipe();
    test_span_wipe();
    test_partial_wipe();
    test_null_and_zero_size();

    if (failures != 0) {
        std::cerr
            << failures
            << " secure-zero test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ secure-zero tests passed.\n";

    return 0;
}