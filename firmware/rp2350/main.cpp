#include "board_init.h"
#include "display_st7796.h"
#include "touch_ft6336u.h"

#include "pico/stdlib.h"

#include <cstdint>
#include <cstdio>

namespace {

constexpr std::uint32_t kInitialLcdSpiHz = 50'000'000;

const char* touch_status_name(
    cryptomachine::hardware::TouchStatus status
) {
    using cryptomachine::hardware::TouchStatus;

    switch (status) {
        case TouchStatus::Success:
            return "success";

        case TouchStatus::NoTouch:
            return "no touch";

        case TouchStatus::InvalidChipId:
            return "invalid chip id";

        case TouchStatus::I2cError:
            return "i2c error";
    }

    return "unknown";
}

}  // namespace

int main() {
    using namespace cryptomachine::hardware;

    const BoardBusRates bus_rates =
        board_init(kInitialLcdSpiHz);

    // Initialize USB stdio after the final board clocks are configured.
    stdio_init_all();
    sleep_ms(1500);

    std::printf("\nCryptoMachine Seed RP2350 bring-up\n");
    std::printf("Board: Waveshare RP2350-Touch-LCD-3.5\n");
    std::printf("MCU package: RP2350B\n");

    std::printf(
        "LCD SPI requested: %lu Hz\n",
        static_cast<unsigned long>(kInitialLcdSpiHz)
    );

    std::printf(
        "LCD SPI actual:    %lu Hz\n",
        static_cast<unsigned long>(bus_rates.lcd_spi_hz)
    );

    std::printf(
        "I2C actual:        %lu Hz\n",
        static_cast<unsigned long>(bus_rates.i2c_hz)
    );

    std::printf("Initializing ST7796 display...\n");

    if (!display_init(DisplayOrientation::Portrait)) {
        std::printf("Display initialization failed.\n");

        while (true) {
            sleep_ms(1000);
        }
    }

    display_fill(0x0000);
    set_backlight_percent(50);

    std::printf(
        "Display initialized: %u x %u\n",
        static_cast<unsigned int>(display_width()),
        static_cast<unsigned int>(display_height())
    );

    std::printf("Initializing FT6336U touch...\n");

    const TouchStatus touch_status = touch_init();

    std::printf(
        "Touch initialization: %s\n",
        touch_status_name(touch_status)
    );

    std::uint32_t heartbeat = 0;

    while (true) {
        std::printf(
            "heartbeat %lu\n",
            static_cast<unsigned long>(heartbeat++)
        );

        if (touch_status == TouchStatus::Success) {
            TouchPoint point{};

            const TouchStatus read_status =
                touch_read(point);

            if (read_status == TouchStatus::Success) {
                std::printf(
                    "touch x=%u y=%u\n",
                    static_cast<unsigned int>(point.x),
                    static_cast<unsigned int>(point.y)
                );
            }
        }

        sleep_ms(2000);
    }
}