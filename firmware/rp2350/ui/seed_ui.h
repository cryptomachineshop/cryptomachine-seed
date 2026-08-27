#pragma once

namespace cryptomachine {

class SeedAppController;

namespace ui {

void seed_ui_init(
    SeedAppController& app
);

void seed_ui_render();

// Critical-fault cleanup only. Scrubs UI-side secret state,
// removes secret-dependent LVGL label references, and deletes
// the active screen's child objects without navigating through
// the normal product workflow.
void seed_ui_emergency_clear();

}  // namespace ui
}  // namespace cryptomachine
