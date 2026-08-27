#include "display_st7796.h"

#include "board_pins.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include <array>

namespace cryptomachine::hardware {
namespace {

std::uint16_t g_width = static_cast<std::uint16_t>(kDisplayWidth);
std::uint16_t g_height = static_cast<std::uint16_t>(kDisplayHeight);

void select_display() {
    gpio_put(kLcdChipSelectPin, 0);
}

void deselect_display() {
    gpio_put(kLcdChipSelectPin, 1);
}

void send_command(std::uint8_t command) {
    gpio_put(kLcdDataCommandPin, 0);

    select_display();
    spi_write_blocking(spi0, &command, 1);
    deselect_display();
}

void send_data_byte(std::uint8_t value) {
    gpio_put(kLcdDataCommandPin, 1);

    select_display();
    spi_write_blocking(spi0, &value, 1);
    deselect_display();
}

void hardware_reset() {
    gpio_put(kLcdResetPin, 1);
    sleep_ms(100);

    gpio_put(kLcdResetPin, 0);
    sleep_ms(100);

    gpio_put(kLcdResetPin, 1);
    sleep_ms(100);
}

void initialize_registers() {
    // RGB565, 16 bits per pixel.
    send_command(0x3A);
    send_data_byte(0x05);

    send_command(0xF0);
    send_data_byte(0xC3);

    send_command(0xF0);
    send_data_byte(0x96);

    send_command(0xB4);
    send_data_byte(0x01);

    send_command(0xB7);
    send_data_byte(0xC6);

    send_command(0xC0);
    send_data_byte(0x80);
    send_data_byte(0x45);

    send_command(0xC1);
    send_data_byte(0x13);

    send_command(0xC2);
    send_data_byte(0xA7);

    send_command(0xC5);
    send_data_byte(0x0A);

    send_command(0xE8);
    send_data_byte(0x40);
    send_data_byte(0x8A);
    send_data_byte(0x00);
    send_data_byte(0x00);
    send_data_byte(0x29);
    send_data_byte(0x19);
    send_data_byte(0xA5);
    send_data_byte(0x33);

    // Positive gamma.
    send_command(0xE0);
    send_data_byte(0xD0);
    send_data_byte(0x08);
    send_data_byte(0x0F);
    send_data_byte(0x06);
    send_data_byte(0x06);
    send_data_byte(0x33);
    send_data_byte(0x30);
    send_data_byte(0x33);
    send_data_byte(0x47);
    send_data_byte(0x17);
    send_data_byte(0x13);
    send_data_byte(0x13);
    send_data_byte(0x2B);
    send_data_byte(0x31);

    // Negative gamma.
    send_command(0xE1);
    send_data_byte(0xD0);
    send_data_byte(0x0A);
    send_data_byte(0x11);
    send_data_byte(0x0B);
    send_data_byte(0x09);
    send_data_byte(0x07);
    send_data_byte(0x2F);
    send_data_byte(0x33);
    send_data_byte(0x47);
    send_data_byte(0x38);
    send_data_byte(0x15);
    send_data_byte(0x16);
    send_data_byte(0x2C);
    send_data_byte(0x32);

    send_command(0xF0);
    send_data_byte(0x3C);

    send_command(0xF0);
    send_data_byte(0x69);

    sleep_ms(120);

    // Display inversion on, matching Waveshare reference.
    send_command(0x21);

    // Display on.
    send_command(0x29);
}

void set_orientation(DisplayOrientation orientation) {
    // Sleep out.
    send_command(0x11);
    sleep_ms(120);

    send_command(0x36);

    if (orientation == DisplayOrientation::Landscape) {
        g_width = 480;
        g_height = 320;
        send_data_byte(0x28);
    } else {
        g_width = 320;
        g_height = 480;
        send_data_byte(0x48);
    }
}

void send_coordinate(std::uint16_t value) {
    send_data_byte(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    send_data_byte(static_cast<std::uint8_t>(value & 0xFF));
}

}  // namespace

bool display_init(DisplayOrientation orientation) {
    hardware_reset();
    set_orientation(orientation);
    initialize_registers();

    return true;
}

std::uint16_t display_width() {
    return g_width;
}

std::uint16_t display_height() {
    return g_height;
}

bool display_set_window(
    std::uint16_t x_start,
    std::uint16_t y_start,
    std::uint16_t x_end,
    std::uint16_t y_end
) {
    if (
        x_start >= x_end ||
        y_start >= y_end ||
        x_end > g_width ||
        y_end > g_height
    ) {
        return false;
    }

    const std::uint16_t x_last =
        static_cast<std::uint16_t>(x_end - 1);

    const std::uint16_t y_last =
        static_cast<std::uint16_t>(y_end - 1);

    // Column address set.
    send_command(0x2A);
    send_coordinate(x_start);
    send_coordinate(x_last);

    // Row address set.
    send_command(0x2B);
    send_coordinate(y_start);
    send_coordinate(y_last);

    // Memory write.
    send_command(0x2C);

    return true;
}

void display_write_pixels(
    const std::uint16_t* pixels,
    std::size_t pixel_count
) {
    if (pixels == nullptr || pixel_count == 0) {
        return;
    }

    // Convert native little-endian uint16_t RGB565 pixels to the
    // big-endian byte order expected by the ST7796.
    std::array<std::uint8_t, 128> buffer{};

    gpio_put(kLcdDataCommandPin, 1);
    select_display();

    std::size_t position = 0;

    while (position < pixel_count) {
        const std::size_t remaining = pixel_count - position;

        const std::size_t chunk_pixels =
            remaining < (buffer.size() / 2)
                ? remaining
                : (buffer.size() / 2);

        for (std::size_t i = 0; i < chunk_pixels; ++i) {
            const std::uint16_t pixel = pixels[position + i];

            buffer[i * 2] =
                static_cast<std::uint8_t>((pixel >> 8) & 0xFF);

            buffer[i * 2 + 1] =
                static_cast<std::uint8_t>(pixel & 0xFF);
        }

        spi_write_blocking(
            spi0,
            buffer.data(),
            chunk_pixels * 2
        );

        position += chunk_pixels;
    }

    deselect_display();
}

void display_write_bytes(
    const std::uint8_t* data,
    std::size_t byte_count
) {
    if (data == nullptr || byte_count == 0) {
        return;
    }

    gpio_put(kLcdDataCommandPin, 1);
    select_display();

    spi_write_blocking(
        spi0,
        data,
        byte_count
    );

    deselect_display();
}

void display_fill(std::uint16_t color) {
    if (!display_set_window(0, 0, g_width, g_height)) {
        return;
    }

    constexpr std::size_t kChunkPixels = 64;
    std::array<std::uint16_t, kChunkPixels> pixels{};

    pixels.fill(color);

    std::size_t remaining =
        static_cast<std::size_t>(g_width) *
        static_cast<std::size_t>(g_height);

    while (remaining > 0) {
        const std::size_t count =
            remaining < pixels.size()
                ? remaining
                : pixels.size();

        display_write_pixels(pixels.data(), count);
        remaining -= count;
    }
}

}  // namespace cryptomachine::hardware