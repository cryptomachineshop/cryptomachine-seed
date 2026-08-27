#pragma once

namespace cryptomachine::ui {

// Creates the temporary hardware bring-up screen used to verify
// LVGL rendering and touchscreen input.
//
// This screen contains no seed-generation functionality or secrets.
void create_bringup_screen();

}  // namespace cryptomachine::ui