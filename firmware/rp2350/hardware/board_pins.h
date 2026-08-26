#pragma once

#include <cstdint>

namespace cryptomachine::hardware {

// ------------------------------------------------------------
// DISPLAY
// Waveshare RP2350-Touch-LCD-3.5
// ST7796, native portrait 320 x 480
// ------------------------------------------------------------

constexpr std::uint32_t kDisplayWidth = 320;
constexpr std::uint32_t kDisplayHeight = 480;

constexpr unsigned int kLcdResetPin = 23;
constexpr unsigned int kLcdDataCommandPin = 20;
constexpr unsigned int kLcdBacklightPin = 22;

constexpr unsigned int kLcdChipSelectPin = 21;
constexpr unsigned int kLcdClockPin = 18;
constexpr unsigned int kLcdMosiPin = 19;

// LCD uses SPI0.

// ------------------------------------------------------------
// TOUCH / SHARED I2C
// FT6336U capacitive touch controller
// ------------------------------------------------------------

constexpr unsigned int kI2cSdaPin = 34;
constexpr unsigned int kI2cSclPin = 35;

// Shared peripheral bus uses I2C1.
constexpr std::uint32_t kI2cBaudHz = 400'000;

constexpr unsigned int kTouchResetPin = 24;
constexpr unsigned int kTouchInterruptPin = 25;

constexpr std::uint8_t kTouchI2cAddress = 0x38;
constexpr std::uint8_t kTouchExpectedChipId = 0x64;

// ------------------------------------------------------------
// OTHER BOARD SIGNALS
// Confirmed from Waveshare native C reference.
// Not yet used by CryptoMachine Seed.
// ------------------------------------------------------------

constexpr unsigned int kDofInterrupt1Pin = 14;
constexpr unsigned int kSystemOutPin = 40;

}  // namespace cryptomachine::hardware