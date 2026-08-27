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

const char* lvgl_fault_name(
    cryptomachine::ui::LvglPortFault fault
) {
    using cryptomachine::ui::LvglPortFault;

    switch (fault) {
        case LvglPortFault::None:
            return "none";
        case LvglPortFault::TouchControllerError:
            return "touch controller runtime error";
    }

    return "unknown LVGL runtime error";
}

const char* seed_ui_fault_name(
    cryptomachine::ui::SeedUiFault fault
) {
    using cryptomachine::ui::SeedUiFault;

    switch (fault) {
        case SeedUiFault::None:
            return "none";
        case SeedUiFault::ControllerActionError:
            return "controller action error";
        case SeedUiFault::InvalidSeedResult:
            return "invalid mnemonic application state";
        case SeedUiFault::UnexpectedApplicationState:
            return "unexpected application state";
    }

    return "unknown application error";
}

[[noreturn]] void halt_startup(
    const char* message
) {
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
    const char* fault_name
) {
    cryptomachine::hardware::set_backlight_percent(0);

    app.emergency_destroy_session();
    cryptomachine::ui::seed_ui_emergency_clear();
    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    cryptomachine::hardware::display_fill(0x0000);

    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    std::printf(
        "RUNTIME FAULT: %s\n",
        fault_name != nullptr
            ? fault_name
            : "unknown runtime error"
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

    std::printf("SeedAppController ready.\n");
    std::printf("Board: Waveshare RP2350-Touch-LCD-3.5\n");
    std::printf("MCU package: RP2350B\n");

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

    std::printf("Initializing ST7796 display...\n");

    if (!display_init(DisplayOrientation::Portrait)) {
        halt_startup(
            "Display initialization failed."
        );
    }

    display_fill(0x0000);

    std::printf(
        "Display initialized: %u x %u\n",
        static_cast<unsigned int>(display_width()),
        static_cast<unsigned int>(display_height())
    );

    std::printf("Initializing FT6336U touch...\n");

    const TouchStatus touch_status =
        touch_init();

    std::printf(
        "Touch initialization: %s\n",
        touch_status_name(touch_status)
    );

    if (touch_status != TouchStatus::Success) {
        halt_startup(
            "Touch initialization failed."
        );
    }

    std::printf("Initializing LVGL...\n");

    if (!cryptomachine::ui::lvgl_port_init()) {
        halt_startup(
            "LVGL initialization failed."
        );
    }

    cryptomachine::ui::seed_ui_init(app);

    const auto initial_ui_fault =
        cryptomachine::ui::seed_ui_fault();

    if (
        initial_ui_fault !=
        cryptomachine::ui::SeedUiFault::None
    ) {
        halt_runtime_fault(
            app,
            seed_ui_fault_name(initial_ui_fault)
        );
    }

    set_backlight_percent(50);

    std::printf("CryptoMachine Seed UI ready.\n");

    while (true) {
        const auto port_fault =
            cryptomachine::ui::lvgl_port_process();

        if (
            port_fault !=
            cryptomachine::ui::LvglPortFault::None
        ) {
            halt_runtime_fault(
                app,
                lvgl_fault_name(port_fault)
            );
        }

        const auto ui_fault =
            cryptomachine::ui::seed_ui_fault();

        if (
            ui_fault !=
            cryptomachine::ui::SeedUiFault::None
        ) {
            halt_runtime_fault(
                app,
                seed_ui_fault_name(ui_fault)
            );
        }

        sleep_ms(5);
    }
}
