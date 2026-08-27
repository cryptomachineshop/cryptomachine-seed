#pragma once

namespace cryptomachine::ui {

bool lvgl_port_init();

void lvgl_port_process();

// Explicitly scrub the complete LVGL RGB565 draw buffer.
// The display flush callback also wipes each rendered region
// immediately after its blocking SPI transfer completes.
void lvgl_port_wipe_draw_buffer();

}  // namespace cryptomachine::ui
