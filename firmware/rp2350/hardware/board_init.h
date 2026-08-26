#pragma once

#include <cstdint>

namespace cryptomachine::hardware {

struct BoardBusRates {
    std::uint32_t lcd_spi_hz;
    std::uint32_t i2c_hz;
};

// Initializes the board-level GPIO and buses.
//
// The LCD SPI rate is supplied by the caller so we do not bake an
// unvalidated Waveshare demo speed into the production hardware layer.
//
// The returned rates are the actual frequencies selected by the Pico SDK.
BoardBusRates board_init(std::uint32_t requested_lcd_spi_hz);

// Backlight brightness, clamped to 0..100 percent.
void set_backlight_percent(std::uint8_t percent);

}  // namespace cryptomachine::hardware