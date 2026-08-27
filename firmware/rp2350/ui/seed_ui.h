#pragma once

namespace cryptomachine {

class SeedAppController;

namespace ui {

// Initializes the production CryptoMachine Seed LVGL interface
// and renders the controller's current application state.
void seed_ui_init(
    SeedAppController& app
);

// Re-renders the screen from the controller's current state.
void seed_ui_render();

}  // namespace ui
}  // namespace cryptomachine