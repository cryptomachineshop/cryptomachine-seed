#include "board_init.h"
#include "display_st7796.h"
#include "lvgl_port.h"
#include "seed_app_controller.h"
#include "seed_ui.h"
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

[[noreturn]] void halt_startup(
    const char* message
) {
    // Never leave a dead or partially initialized product UI
    // illuminated after a startup failure.
    cryptomachine::hardware::set_backlight_percent(0);

    std::printf(
        "STARTUP HALTED: %s\n",
        message != nullptr
            ? message
            : "unknown error"
    );

    while (true) {
        sleep_ms(1000);
    }
}

[[noreturn]] void halt_runtime_fault(
    cryptomachine::SeedAppController& app,
    cryptomachine::ui::LvglPortFault fault
) {
    // Hide the panel immediately. Any mnemonic pixels still
    // present in LCD GRAM are no longer visible while cleanup
    // proceeds.
    cryptomachine::hardware::set_backlight_percent(0);

    // Core wipe is unconditional and does not depend on the
    // touchscreen or current UI state being trustworthy.
    app.emergency_destroy_session();

    // Remove UI-side secret references and fixed input buffers.
    cryptomachine::ui::seed_ui_emergency_clear();

    // Scrub rendered mnemonic pixels from RP2350 RAM.
    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    // Remove any old sensitive image from the LCD controller's
    // own display memory as well.
    cryptomachine::hardware::display_fill(0x0000);

    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    const char* fault_name =
        fault ==
            cryptomachine::ui::
                LvglPortFault::TouchControllerError
            ? "touch controller runtime error"
            : "unknown runtime error";

    std::printf(
        "RUNTIME FAULT: %s\n",
        fault_name
    );

    std::printf(
        "Sensitive session destroyed. "
        "Power-cycle required.\n"
    );

    while (true) {
        sleep_ms(1000);
    }
}

}  // namespace

int main() {
    using namespace cryptomachine::hardware;

    cryptomachine::SeedAppController app;

    const BoardBusRates bus_rates =
        board_init(kInitialLcdSpiHz);

    // board_init() already configures the PWM backlight at 0%.
    // Keep the intent explicit at the application boundary too.
    set_backlight_percent(0);

    stdio_init_all();
    sleep_ms(1500);

    std::printf(
        "\nCryptoMachine Seed RP2350 bring-up\n"
    );

    if (
        app.boot_complete() !=
        cryptomachine::SeedAppStatus::Success
    ) {
        halt_startup(
            "SeedAppController boot failed."
        );
    }

    std::printf(
        "SeedAppController ready.\n"
    );

    std::printf(
        "Board: Waveshare RP2350-Touch-LCD-3.5\n"
    );

    std::printf(
        "MCU package: RP2350B\n"
    );

    std::printf(
        "LCD SPI requested: %lu Hz\n",
        static_cast<unsigned long>(
            kInitialLcdSpiHz
        )
    );

    std::printf(
        "LCD SPI actual:    %lu Hz\n",
        static_cast<unsigned long>(
            bus_rates.lcd_spi_hz
        )
    );

    std::printf(
        "I2C actual:        %lu Hz\n",
        static_cast<unsigned long>(
            bus_rates.i2c_hz
        )
    );

    std::printf(
        "Initializing ST7796 display...\n"
    );

    if (
        !display_init(
            DisplayOrientation::Portrait
        )
    ) {
        halt_startup(
            "Display initialization failed."
        );
    }

    // Establish a known visual state before any UI becomes
    // visible. The backlight remains off at this point.
    display_fill(0x0000);

    std::printf(
        "Display initialized: %u x %u\n",
        static_cast<unsigned int>(
            display_width()
        ),
        static_cast<unsigned int>(
            display_height()
        )
    );

    std::printf(
        "Initializing FT6336U touch...\n"
    );

    const TouchStatus touch_status =
        touch_init();

    std::printf(
        "Touch initialization: %s\n",
        touch_status_name(touch_status)
    );

    if (
        touch_status !=
        TouchStatus::Success
    ) {
        halt_startup(
            "Touch initialization failed."
        );
    }

    std::printf(
        "Initializing LVGL...\n"
    );

    if (
        !cryptomachine::ui::lvgl_port_init()
    ) {
        halt_startup(
            "LVGL initialization failed."
        );
    }

    cryptomachine::ui::seed_ui_init(app);

    // Illuminate the display only after the controller,
    // display, touch, LVGL port, and product UI have all
    // initialized successfully.
    set_backlight_percent(50);

    std::printf(
        "CryptoMachine Seed UI ready.\n"
    );

    while (true) {
        const auto runtime_fault =
            cryptomachine::ui::lvgl_port_process();

        if (
            runtime_fault !=
            cryptomachine::ui::LvglPortFault::None
        ) {
            halt_runtime_fault(
                app,
                runtime_fault
            );
        }

        sleep_ms(5);
    }
}
