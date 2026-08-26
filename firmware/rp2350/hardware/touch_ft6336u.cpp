#include "touch_ft6336u.h"

#include "board_pins.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace cryptomachine::hardware {
namespace {

constexpr std::uint8_t kRegisterTouchStatus = 0x02;
constexpr std::uint8_t kRegisterTouch1XHigh = 0x03;
constexpr std::uint8_t kRegisterChipId = 0xA3;
constexpr std::uint8_t kRegisterGestureEnable = 0xD0;

bool write_register(std::uint8_t reg, std::uint8_t value) {
    const std::array<std::uint8_t, 2> data{
        reg,
        value,
    };

    const int result = i2c_write_blocking(
        i2c1,
        kTouchI2cAddress,
        data.data(),
        data.size(),
        false
    );

    return result == static_cast<int>(data.size());
}

bool read_register(
    std::uint8_t reg,
    std::uint8_t* data,
    std::size_t length
) {
    if (data == nullptr || length == 0) {
        return false;
    }

    const int write_result = i2c_write_blocking(
        i2c1,
        kTouchI2cAddress,
        &reg,
        1,
        true
    );

    if (write_result != 1) {
        return false;
    }

    const int read_result = i2c_read_blocking(
        i2c1,
        kTouchI2cAddress,
        data,
        length,
        false
    );

    return read_result == static_cast<int>(length);
}

void reset_controller() {
    // Explicitly configure the reset pin here as well as in board_init().
    // This keeps the touch driver safe if its initialization sequence is
    // ever reused independently.
    gpio_init(kTouchResetPin);
    gpio_set_dir(kTouchResetPin, GPIO_OUT);

    // Waveshare FT6336U reset timing:
    // HIGH 10 ms -> LOW 10 ms -> HIGH 300 ms.
    gpio_put(kTouchResetPin, 1);
    sleep_ms(10);

    gpio_put(kTouchResetPin, 0);
    sleep_ms(10);

    gpio_put(kTouchResetPin, 1);
    sleep_ms(300);
}

std::uint16_t decode_coordinate(
    std::uint8_t high,
    std::uint8_t low
) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high & 0x0F) << 8) |
        static_cast<std::uint16_t>(low)
    );
}

}  // namespace

TouchStatus touch_init() {
    // Waveshare uses the interrupt line as an input with pull-up.
    gpio_init(kTouchInterruptPin);
    gpio_set_dir(kTouchInterruptPin, GPIO_IN);
    gpio_pull_up(kTouchInterruptPin);

    reset_controller();

    // CryptoMachine Seed uses point mode only.
    // Disable FT6336U gesture processing.
    if (!write_register(kRegisterGestureEnable, 0x00)) {
        return TouchStatus::I2cError;
    }

    std::uint8_t chip_id = 0;

    if (!read_register(kRegisterChipId, &chip_id, 1)) {
        return TouchStatus::I2cError;
    }

    if (chip_id != kTouchExpectedChipId) {
        return TouchStatus::InvalidChipId;
    }

    return TouchStatus::Success;
}

TouchStatus touch_read(TouchPoint& point) {
    std::uint8_t touch_status = 0;

    if (!read_register(kRegisterTouchStatus, &touch_status, 1)) {
        return TouchStatus::I2cError;
    }

    // FT6336U TD_STATUS uses the low nibble for the number of
    // currently detected touch points.
    const std::uint8_t touch_count =
        static_cast<std::uint8_t>(touch_status & 0x0F);

    if (touch_count == 0) {
        return TouchStatus::NoTouch;
    }

    // Registers 0x03..0x06 contain:
    //   TOUCH1_XH
    //   TOUCH1_XL
    //   TOUCH1_YH
    //   TOUCH1_YL
    std::array<std::uint8_t, 4> coordinates{};

    if (
        !read_register(
            kRegisterTouch1XHigh,
            coordinates.data(),
            coordinates.size()
        )
    ) {
        return TouchStatus::I2cError;
    }

    std::uint16_t x =
        decode_coordinate(coordinates[0], coordinates[1]);

    std::uint16_t y =
        decode_coordinate(coordinates[2], coordinates[3]);

    // Waveshare's portrait implementation maps FT6336U X/Y directly
    // to the native 320 x 480 display coordinates.
    //
    // Bound unexpected controller values so callers can never receive
    // coordinates outside the physical display.
    if (x >= kDisplayWidth) {
        x = static_cast<std::uint16_t>(kDisplayWidth - 1);
    }

    if (y >= kDisplayHeight) {
        y = static_cast<std::uint16_t>(kDisplayHeight - 1);
    }

    point.x = x;
    point.y = y;

    return TouchStatus::Success;
}

bool touch_interrupt_active() {
    // Waveshare's reference example reacts to a rising edge on GPIO 25,
    // so preserve that observed behavior here rather than assuming an
    // undocumented inverted polarity.
    return gpio_get(kTouchInterruptPin) != 0;
}

}  // namespace cryptomachine::hardware