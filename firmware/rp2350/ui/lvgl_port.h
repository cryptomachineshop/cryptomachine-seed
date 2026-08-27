#pragma once

namespace cryptomachine::ui {

enum class LvglPortFault {
    None = 0,
    TouchControllerError,
};

bool lvgl_port_init();

// Processes one LVGL iteration and returns any latched
// critical runtime hardware fault. NoTouch is normal and
// never produces a fault.
LvglPortFault lvgl_port_process();

// Returns and clears the touch-activity latch. A successful
// physical touch read counts as activity even when that touch
// is intentionally suppressed by the inactivity wake guard.
bool lvgl_port_consume_touch_activity();

// Arms a wake-only guard for the next physical touch.
// The first touch is reported to inactivity handling but is
// not forwarded to LVGL as a press. Input remains suppressed
// until the user physically releases the screen, preventing
// the wake touch from becoming an accidental button click.
void lvgl_port_arm_wake_guard();

// Clears all wake-guard state. Intended for session teardown
// or a return to a non-sensitive Home state.
void lvgl_port_cancel_wake_guard();

// Explicitly scrub the complete LVGL RGB565 draw buffer.
// The display flush callback also wipes each rendered region
// immediately after its blocking SPI transfer completes.
void lvgl_port_wipe_draw_buffer();

}  // namespace cryptomachine::ui
