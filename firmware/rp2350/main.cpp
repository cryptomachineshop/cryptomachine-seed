#include "board_init.h"
#include "display_st7796.h"
#include "dev_log.h"
#include "inactivity_policy.h"
#include "lvgl_port.h"
#include "seed_app_controller.h"
#include "seed_ui.h"
#include "touch_ft6336u.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include <cstdint>

namespace {

constexpr std::uint32_t kInitialLcdSpiHz = 50'000'000;

constexpr std::uint8_t kActiveBacklightPercent = 50;
constexpr std::uint8_t kDimBacklightPercent = 10;
constexpr std::uint8_t kWarningBacklightPercent = 30;

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

    CM_DEV_LOG(
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

    CM_DEV_LOG(
        "RUNTIME FAULT: %s\n",
        fault_name != nullptr
            ? fault_name
            : "unknown runtime error"
    );

    CM_DEV_LOG(
        "Sensitive session destroyed. "
        "Power-cycle required.\n"
    );

    while (true) {
        sleep_ms(1000);
    }
}

void finalize_destroyed_session_display() {
    // The controller must already contain no sensitive session
    // material before this function is called.
    //
    // Physically darken first, remove all UI-side references,
    // scrub LVGL pixels, overwrite controller GRAM, then build
    // the non-sensitive Home screen.
    cryptomachine::hardware::set_backlight_percent(0);

    cryptomachine::ui::seed_ui_emergency_clear();
    cryptomachine::ui::lvgl_port_wipe_draw_buffer();

    cryptomachine::hardware::display_fill(0x0000);

    cryptomachine::ui::lvgl_port_wipe_draw_buffer();
    cryptomachine::ui::lvgl_port_cancel_wake_guard();

    cryptomachine::ui::seed_ui_render();

    cryptomachine::hardware::set_backlight_percent(
        kActiveBacklightPercent
    );
}

void destroy_session_for_inactivity(
    cryptomachine::SeedAppController& app
) {
    // Timeout destruction is unconditional because no UI state
    // transition can be trusted to have been requested by a user.
    app.emergency_destroy_session();

    finalize_destroyed_session_display();
}

void apply_inactivity_decision(
    cryptomachine::SeedAppController& app,
    const cryptomachine::InactivityDecision& decision
) {
    using cryptomachine::InactivityEvent;

    switch (decision.event) {
        case InactivityEvent::None:
            return;

        case InactivityEvent::EnterDim:
            cryptomachine::hardware::set_backlight_percent(
                kDimBacklightPercent
            );

            // The first physical touch after dim is wake-only.
            cryptomachine::ui::lvgl_port_arm_wake_guard();
            return;

        case InactivityEvent::EnterWarning:
            // Never merely cover mnemonic pixels. Darken the panel,
            // wipe the LVGL buffer, overwrite LCD GRAM, then build
            // the generic warning.
            cryptomachine::hardware::set_backlight_percent(0);

            cryptomachine::ui::lvgl_port_arm_wake_guard();
            cryptomachine::ui::lvgl_port_wipe_draw_buffer();

            cryptomachine::hardware::display_fill(0x0000);

            cryptomachine::ui::lvgl_port_wipe_draw_buffer();

            cryptomachine::ui::seed_ui_show_inactivity_warning(
                decision.warning_seconds
            );

            cryptomachine::hardware::set_backlight_percent(
                kWarningBacklightPercent
            );
            return;

        case InactivityEvent::WarningTick:
            cryptomachine::ui::seed_ui_show_inactivity_warning(
                decision.warning_seconds
            );
            return;

        case InactivityEvent::WakeFromDim:
            cryptomachine::hardware::set_backlight_percent(
                kActiveBacklightPercent
            );

            // Do not cancel the port wake guard here. The first
            // physical touch has been consumed, and suppression
            // must remain until the finger is actually released.
            return;

        case InactivityEvent::WakeFromWarning:
            // Hide the generic warning while reconstructing the
            // exact controller/UI state.
            cryptomachine::hardware::set_backlight_percent(0);

            cryptomachine::ui::seed_ui_render();

            cryptomachine::hardware::set_backlight_percent(
                kActiveBacklightPercent
            );

            // As above, keep suppression until physical release.
            return;

        case InactivityEvent::Expire:
            destroy_session_for_inactivity(app);
            return;

        case InactivityEvent::HomeReset:
            cryptomachine::ui::lvgl_port_cancel_wake_guard();

            // Re-rendering Home is harmless and guarantees that a
            // generic inactivity warning cannot remain visible if
            // controller state changed to Home while inactive.
            cryptomachine::ui::seed_ui_render();

            cryptomachine::hardware::set_backlight_percent(
                kActiveBacklightPercent
            );
            return;
    }
}

}  // namespace

int main() {
    using namespace cryptomachine::hardware;

    cryptomachine::SeedAppController app;

    const BoardBusRates bus_rates =
        board_init(kInitialLcdSpiHz);

    set_backlight_percent(0);

#if CRYPTOMACHINE_DEV_LOGGING
    // USB stdio and its enumeration delay exist only in explicit
    // development builds. Production begins immediately with no
    // first-party diagnostic transport initialized.
    stdio_init_all();
    sleep_ms(1500);
#endif

    CM_DEV_LOG(
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

    CM_DEV_LOG("SeedAppController ready.\n");
    CM_DEV_LOG("Board: Waveshare RP2350-Touch-LCD-3.5\n");
    CM_DEV_LOG("MCU package: RP2350B\n");

    CM_DEV_LOG(
        "LCD SPI requested: %lu Hz\n",
        static_cast<unsigned long>(
            kInitialLcdSpiHz
        )
    );

    CM_DEV_LOG(
        "LCD SPI actual:    %lu Hz\n",
        static_cast<unsigned long>(
            bus_rates.lcd_spi_hz
        )
    );

    CM_DEV_LOG(
        "I2C actual:        %lu Hz\n",
        static_cast<unsigned long>(
            bus_rates.i2c_hz
        )
    );

    CM_DEV_LOG("Initializing ST7796 display...\n");

    if (!display_init(DisplayOrientation::Portrait)) {
        halt_startup(
            "Display initialization failed."
        );
    }

    display_fill(0x0000);

    CM_DEV_LOG(
        "Display initialized: %u x %u\n",
        static_cast<unsigned int>(display_width()),
        static_cast<unsigned int>(display_height())
    );

    CM_DEV_LOG("Initializing FT6336U touch...\n");

    const TouchStatus touch_status =
        touch_init();

    CM_DEV_LOG(
        "Touch initialization: %s\n",
        touch_status_name(touch_status)
    );

    if (touch_status != TouchStatus::Success) {
        halt_startup(
            "Touch initialization failed."
        );
    }

    CM_DEV_LOG("Initializing LVGL...\n");

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

    CM_DEV_LOG("CryptoMachine Seed UI ready.\n");

    cryptomachine::InactivityPolicy inactivity;
    inactivity.reset(monotonic_ms());

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

        if (
            cryptomachine::ui::
                seed_ui_consume_destroy_display_clear_request()
        ) {
            // Normal user-confirmed controller destruction has
            // already completed. Finish the physical/UI wipe now,
            // safely outside the LVGL event callback.
            finalize_destroyed_session_display();

            inactivity.reset(monotonic_ms());

            sleep_ms(5);
            continue;
        }

        const bool touch_activity =
            cryptomachine::ui::
                lvgl_port_consume_touch_activity();

        const auto decision =
            inactivity.update(
                monotonic_ms(),
                app.state() ==
                    cryptomachine::UIState::Home,
                touch_activity
            );

        apply_inactivity_decision(
            app,
            decision
        );

        sleep_ms(5);
    }
}
