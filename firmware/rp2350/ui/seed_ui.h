#pragma once

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

// Returns the first latched UI/application fault. Faults remain
// latched until reboot; main() consumes them and performs the
// unconditional emergency-destruction path.
SeedUiFault seed_ui_fault();

// Critical-fault cleanup only. Scrubs UI-side secret state,
// removes secret-dependent LVGL label references, and deletes
// the active screen's child objects without navigating through
// the normal product workflow.
void seed_ui_emergency_clear();

}  // namespace ui
}  // namespace cryptomachine
