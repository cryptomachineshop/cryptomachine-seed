#pragma once

namespace cryptomachine::ui {

// Initializes LVGL and connects it to the CryptoMachine
// ST7796 display and FT6336U touch drivers.
//
// Hardware display and touch initialization must already
// have completed successfully.
bool lvgl_port_init();

// Run LVGL's timer/task processing.
//
// Call this repeatedly from the application's main loop.
void lvgl_port_process();

}  // namespace cryptomachine::ui