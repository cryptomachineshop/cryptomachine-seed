#pragma once

#include <cstdint>

namespace cryptomachine {

class SeedAppController;

namespace ui {

enum class SeedUiFault {
    None = 0,
    ControllerActionError,
    InvalidSeedResult,
    UnexpectedApplicationState,
};

void seed_ui_init(
    SeedAppController& app
);

void seed_ui_render();

// Replaces the current ceremony view with a generic inactivity
// warning without modifying controller state or UI-side partial
// dice entry. Calling seed_ui_render() restores the exact
// ceremony state after a wake touch.
void seed_ui_show_inactivity_warning(
    std::uint32_t remaining_seconds
);

// Returns true exactly once after a user-confirmed normal session
// destruction succeeds. main() uses this to perform the physical
// backlight-off/LCD-GRAM wipe outside the LVGL event callback.
bool seed_ui_consume_destroy_display_clear_request();

// Returns the first latched UI/application fault. Faults remain
// latched until reboot; main() consumes them and performs the
// unconditional emergency-destruction path.
SeedUiFault seed_ui_fault();

// Critical-fault/forced-destruction cleanup. Scrubs UI-side
// secret state, removes secret-dependent LVGL label references,
// and deletes the active screen's child objects without
// navigating through the normal product workflow.
void seed_ui_emergency_clear();

}  // namespace ui
}  // namespace cryptomachine
