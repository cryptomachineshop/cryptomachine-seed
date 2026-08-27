#pragma once

#include <cstddef>
#include <cstdint>

namespace cryptomachine::hardware {

enum class DisplayOrientation : std::uint8_t {
    Portrait = 0,
    Landscape,
};

// Initialize the ST7796 display controller.
//
// The backlight is not enabled here. Board initialization should leave
// the backlight off until display initialization has completed.
bool display_init(
    DisplayOrientation orientation = DisplayOrientation::Portrait
);

std::uint16_t display_width();
std::uint16_t display_height();

// Set an active drawing window.
//
// x_end and y_end are exclusive, so:
//   set_window(0, 0, 320, 480)
// addresses the entire portrait display.
bool display_set_window(
    std::uint16_t x_start,
    std::uint16_t y_start,
    std::uint16_t x_end,
    std::uint16_t y_end
);

// Write RGB565 pixels into the currently selected window.
// Each uint16_t is transmitted most-significant byte first.
void display_write_pixels(
    const std::uint16_t* pixels,
    std::size_t pixel_count
);

// Write bytes exactly as supplied.
//
// Intended for display pipelines such as LVGL where RGB565 byte ordering
// has already been prepared by the renderer.
void display_write_bytes(
    const std::uint8_t* data,
    std::size_t byte_count
);

// Fill the entire display with one RGB565 color.
void display_fill(std::uint16_t color);

}  // namespace cryptomachine::hardware