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

// Explicitly scrub the complete LVGL RGB565 draw buffer.
// The display flush callback also wipes each rendered region
// immediately after its blocking SPI transfer completes.
void lvgl_port_wipe_draw_buffer();

}  // namespace cryptomachine::ui
