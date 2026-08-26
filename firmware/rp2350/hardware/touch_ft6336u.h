#pragma once

#include <cstdint>

namespace cryptomachine::hardware {

struct TouchPoint {
    std::uint16_t x;
    std::uint16_t y;
};

enum class TouchStatus : std::uint8_t {
    Success = 0,
    NoTouch,
    InvalidChipId,
    I2cError,
};

// Reset the FT6336U and verify its chip ID.
//
// Uses point mode only. Gesture mode is intentionally not enabled for
// CryptoMachine Seed.
TouchStatus touch_init();

// Read the first active touch point.
//
// Portrait orientation uses the FT6336U raw X/Y coordinates directly,
// matching Waveshare's native 320 x 480 reference implementation.
TouchStatus touch_read(TouchPoint& point);

// Returns true while the controller's interrupt line is asserted.
// This does not perform any I2C access.
bool touch_interrupt_active();

}  // namespace cryptomachine::hardware