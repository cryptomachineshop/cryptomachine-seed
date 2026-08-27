#include "board_init.h"
#include "display_st7796.h"
#include "lvgl_port.h"
#include "seed_app_controller.h"
#include "seed_ui.h"
#include "touch_ft6336u.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include <cstdint>
#include <cstdio>

namespace {

constexpr std::uint32_t kInitialLcdSpiHz = 50'000'000;

constexpr std::uint8_t kActiveBacklightPercent = 50;
constexpr std::uint8_t kDimBacklightPercent = 10;
constexpr std::uint8_t kWarningBacklightPercent = 30;

constexpr std::uint64_t kMillisecondsPerSecond = 1'000;
constexpr std::uint64_t kMillisecondsPerMinute =
    60 * kMillisecondsPerSecond;

// Product inactivity policy:
//   3:00 idle -> dim, preserve ceremony
//   9:00 idle -> hide ceremony, show 60-second warning
//  10:00 idle -> securely destroy session and return Home
constexpr std::uint64_t kDimAfterMs =
    3 * kMillisecondsPerMinute;

constexpr std::uint64_t kWarningAfterMs =
    9 * kMillisecondsPerMinute;

constexpr std::uint64_t kWipeAfterMs =
    10 * kMillisecondsPerMinute;

enum class InactivityState {
    Active = 0,
    Dimmed,
    Warning,
};

std::uint64_t monotonic_ms() {
    return time_us_64() / 1'000ULL;
}

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

void destroy_session_for_inactivity(
    cryptomachine::SeedAppController& app
) {
    // Make the secret display physically dark before touching
    // session state or constructing the Home screen.
    cryptomachine::hardware::set_backlight_percent(0);

    app.emergency_destroy_session();

    cryptomachine::ui::seed_ui_emergency_clear();
    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    // Overwrite ST7796 GRAM so mnemonic/dice pixels cannot
    // survive behind the next LVGL screen.
    cryptomachine::hardware::display_fill(0x0000);

    cryptomachine::ui::lvgl_port_wipe_draw_buffer();
    cryptomachine::ui::lvgl_port_cancel_wake_guard();

    // emergency_destroy_session() reset the controller to Home.
    cryptomachine::ui::seed_ui_render();

    cryptomachine::hardware::set_backlight_percent(
        kActiveBacklightPercent
    );
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

    set_backlight_percent(
        kActiveBacklightPercent
    );

    std::printf("CryptoMachine Seed UI ready.\n");

    std::uint64_t last_activity_ms =
        monotonic_ms();

    InactivityState inactivity_state =
        InactivityState::Active;

    std::uint32_t last_warning_seconds = 0;

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

        const std::uint64_t now_ms =
            monotonic_ms();

        const bool touch_activity =
            cryptomachine::ui::
                lvgl_port_consume_touch_activity();

        // Home has no seed ceremony to protect. Keep the normal
        // UI awake and restart the inactivity clock there.
        if (
            app.state() ==
            cryptomachine::UIState::Home
        ) {
            last_activity_ms = now_ms;

            if (
                inactivity_state !=
                InactivityState::Active
            ) {
                cryptomachine::ui::
                    lvgl_port_cancel_wake_guard();

                if (
                    inactivity_state ==
                    InactivityState::Warning
                ) {
                    cryptomachine::ui::
                        seed_ui_render();
                }

                set_backlight_percent(
                    kActiveBacklightPercent
                );

                inactivity_state =
                    InactivityState::Active;

                last_warning_seconds = 0;
            }

            sleep_ms(5);
            continue;
        }

        if (touch_activity) {
            last_activity_ms = now_ms;

            if (
                inactivity_state ==
                InactivityState::Warning
            ) {
                // Hide the generic warning while the real
                // ceremony view is reconstructed.
                set_backlight_percent(0);

                cryptomachine::ui::
                    seed_ui_render();
            }

            set_backlight_percent(
                kActiveBacklightPercent
            );

            inactivity_state =
                InactivityState::Active;

            last_warning_seconds = 0;

            // Do NOT cancel the port wake guard here. The port
            // has already consumed this first touch and must keep
            // suppressing input until physical release.
        }

        const std::uint64_t elapsed_ms =
            now_ms - last_activity_ms;

        if (elapsed_ms >= kWipeAfterMs) {
            destroy_session_for_inactivity(app);

            last_activity_ms = now_ms;

            inactivity_state =
                InactivityState::Active;

            last_warning_seconds = 0;

            sleep_ms(5);
            continue;
        }

        if (elapsed_ms >= kWarningAfterMs) {
            const std::uint64_t remaining_ms =
                kWipeAfterMs - elapsed_ms;

            std::uint32_t remaining_seconds =
                static_cast<std::uint32_t>(
                    (
                        remaining_ms +
                        kMillisecondsPerSecond - 1
                    ) /
                    kMillisecondsPerSecond
                );

            if (remaining_seconds == 0) {
                remaining_seconds = 1;
            }

            if (
                inactivity_state !=
                InactivityState::Warning
            ) {
                // The warning must never merely cover mnemonic
                // pixels. Darken the panel and overwrite GRAM
                // before constructing the generic warning.
                set_backlight_percent(0);

                cryptomachine::ui::
                    lvgl_port_arm_wake_guard();

                cryptomachine::ui::
                    lvgl_port_wipe_draw_buffer();

                display_fill(0x0000);

                cryptomachine::ui::
                    lvgl_port_wipe_draw_buffer();

                cryptomachine::ui::
                    seed_ui_show_inactivity_warning(
                        remaining_seconds
                    );

                set_backlight_percent(
                    kWarningBacklightPercent
                );

                inactivity_state =
                    InactivityState::Warning;

                last_warning_seconds =
                    remaining_seconds;
            } else if (
                remaining_seconds !=
                last_warning_seconds
            ) {
                cryptomachine::ui::
                    seed_ui_show_inactivity_warning(
                        remaining_seconds
                    );

                last_warning_seconds =
                    remaining_seconds;
            }
        } else if (
            elapsed_ms >= kDimAfterMs &&
            inactivity_state ==
                InactivityState::Active
        ) {
            set_backlight_percent(
                kDimBacklightPercent
            );

            // From this point onward, the first physical touch
            // is wake-only and cannot activate the button that
            // happened to be underneath the user's finger.
            cryptomachine::ui::
                lvgl_port_arm_wake_guard();

            inactivity_state =
                InactivityState::Dimmed;
        }

        sleep_ms(5);
    }
}
