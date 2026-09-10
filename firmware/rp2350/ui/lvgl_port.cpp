#include "lvgl_port.h"

#include "board_pins.h"
#include "display_st7796.h"
#include "secure_zero.h"
#include "touch_ft6336u.h"

#include "lvgl.h"
#include "pico/time.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace cryptomachine::ui {
namespace {

constexpr std::size_t kDrawBufferRows = 20;

std::array<
    lv_color_t,
    cryptomachine::hardware::kDisplayWidth * kDrawBufferRows
> g_draw_buffer{};

lv_disp_draw_buf_t g_display_buffer{};
lv_disp_drv_t g_display_driver{};
lv_indev_drv_t g_touch_driver{};

repeating_timer g_tick_timer{};

cryptomachine::hardware::TouchPoint g_last_touch{};

bool g_initialized = false;

LvglPortFault g_runtime_fault =
    LvglPortFault::None;

bool g_touch_activity = false;
bool g_wake_guard_armed = false;
bool g_suppress_until_release = false;

bool tick_callback(repeating_timer*) {
    lv_tick_inc(5);
    return true;
}

void wipe_pixel_buffer(
    lv_color_t* color_buffer,
    std::size_t pixel_count
) {
    if (
        color_buffer == nullptr ||
        pixel_count == 0
    ) {
        return;
    }

    secure_zero(
        color_buffer,
        pixel_count * sizeof(lv_color_t)
    );
}

void display_flush_callback(
    lv_disp_drv_t* driver,
    const lv_area_t* area,
    lv_color_t* color_buffer
) {
    if (
        driver == nullptr ||
        area == nullptr ||
        color_buffer == nullptr
    ) {
        if (driver != nullptr) {
            lv_disp_flush_ready(driver);
        }
        return;
    }

    const std::size_t width =
        static_cast<std::size_t>(
            area->x2 - area->x1 + 1
        );

    const std::size_t height =
        static_cast<std::size_t>(
            area->y2 - area->y1 + 1
        );

    const std::size_t pixel_count =
        width * height;

    const std::uint16_t x_start =
        static_cast<std::uint16_t>(
            area->x1
        );

    const std::uint16_t y_start =
        static_cast<std::uint16_t>(
            area->y1
        );

    const std::uint16_t x_end =
        static_cast<std::uint16_t>(
            area->x2 + 1
        );

    const std::uint16_t y_end =
        static_cast<std::uint16_t>(
            area->y2 + 1
        );
    if (
        !cryptomachine::hardware::display_write_window_bytes(
            x_start,
            y_start,
            x_end,
            y_end,
            reinterpret_cast<const std::uint8_t*>(
                color_buffer
            ),
            pixel_count * sizeof(lv_color_t)
        )
    ) {
        // LVGL may already have rendered sensitive pixels into the
        // draw buffer even when the display transfer cannot proceed.
        wipe_pixel_buffer(
            color_buffer,
            pixel_count
        );

        lv_disp_flush_ready(driver);
        return;
    }

// Do not scrub the active LVGL draw buffer before flush-ready.
    // Hardware testing confirmed that overwriting it at this point
    // corrupts subsequent rendering.
    //
    // Sensitive pixels are scrubbed at explicit security boundaries
    // such as session destruction, inactivity expiry, and fatal-fault
    // handling. The abandoned-transfer failure path above is still
    // scrubbed immediately.
    lv_disp_flush_ready(driver);
}

void touch_read_callback(
    lv_indev_drv_t*,
    lv_indev_data_t* data
) {
    if (data == nullptr) {
        return;
    }

    // Once a critical input fault is latched, never touch the
    // I2C controller again. Report PRESSED rather than RELEASED
    // so LVGL cannot synthesize a click from a failed read.
    if (
        g_runtime_fault !=
        LvglPortFault::None
    ) {
        data->point.x =
            static_cast<lv_coord_t>(
                g_last_touch.x
            );

        data->point.y =
            static_cast<lv_coord_t>(
                g_last_touch.y
            );

        data->state =
            LV_INDEV_STATE_PRESSED;

        return;
    }

    cryptomachine::hardware::TouchPoint point{};

    const auto status =
        cryptomachine::hardware::touch_read(
            point
        );

    if (
        status ==
        cryptomachine::hardware::TouchStatus::Success
    ) {
        g_last_touch = point;
        g_touch_activity = true;

        data->point.x =
            static_cast<lv_coord_t>(
                point.x
            );

        data->point.y =
            static_cast<lv_coord_t>(
                point.y
            );

        if (g_wake_guard_armed) {
            // Consume the first real touch as wake-only.
            // Keep suppressing until physical release so a
            // held finger cannot turn into a CLICKED event.
            g_wake_guard_armed = false;
            g_suppress_until_release = true;
        }

        data->state =
            g_suppress_until_release
                ? LV_INDEV_STATE_RELEASED
                : LV_INDEV_STATE_PRESSED;

        return;
    }

    data->point.x =
        static_cast<lv_coord_t>(
            g_last_touch.x
        );

    data->point.y =
        static_cast<lv_coord_t>(
            g_last_touch.y
        );

    if (
        status ==
        cryptomachine::hardware::TouchStatus::NoTouch
    ) {
        // Normal inactivity. Preserve the current ceremony.
        data->state =
            LV_INDEV_STATE_RELEASED;

        if (g_suppress_until_release) {
            // The wake touch has now been physically released.
            // The next touch may interact with the restored UI.
            g_suppress_until_release = false;
        }

        return;
    }

    // Any actual touch-controller communication/state failure
    // is fatal to the current appliance session. Keep the
    // input logically pressed so a failed read cannot become
    // an accidental CLICKED event before main() handles the
    // latched fault.
    g_runtime_fault =
        LvglPortFault::TouchControllerError;

    data->state =
        LV_INDEV_STATE_PRESSED;
}

}  // namespace

bool lvgl_port_init() {
    if (g_initialized) {
        return true;
    }

    g_runtime_fault =
        LvglPortFault::None;

    g_touch_activity = false;
    g_wake_guard_armed = false;
    g_suppress_until_release = false;

    static_assert(
        sizeof(lv_color_t) == 2,
        "CryptoMachine LVGL port requires RGB565"
    );

    // Start from a known clean draw buffer.
    secure_zero(
        g_draw_buffer.data(),
        g_draw_buffer.size() *
            sizeof(g_draw_buffer[0])
    );

    lv_init();

    lv_disp_draw_buf_init(
        &g_display_buffer,
        g_draw_buffer.data(),
        nullptr,
        g_draw_buffer.size()
    );

    lv_disp_drv_init(
        &g_display_driver
    );

    g_display_driver.hor_res =
        static_cast<lv_coord_t>(
            cryptomachine::hardware::kDisplayWidth
        );

    g_display_driver.ver_res =
        static_cast<lv_coord_t>(
            cryptomachine::hardware::kDisplayHeight
        );


    g_display_driver.flush_cb =
        display_flush_callback;

    g_display_driver.draw_buf =
        &g_display_buffer;

    if (
        lv_disp_drv_register(
            &g_display_driver
        ) == nullptr
    ) {
        secure_zero(
            g_draw_buffer.data(),
            g_draw_buffer.size() *
                sizeof(g_draw_buffer[0])
        );

        return false;
    }

    lv_indev_drv_init(
        &g_touch_driver
    );

    g_touch_driver.type =
        LV_INDEV_TYPE_POINTER;

    g_touch_driver.read_cb =
        touch_read_callback;

    if (
        lv_indev_drv_register(
            &g_touch_driver
        ) == nullptr
    ) {
        secure_zero(
            g_draw_buffer.data(),
            g_draw_buffer.size() *
                sizeof(g_draw_buffer[0])
        );

        return false;
    }

    if (
        !add_repeating_timer_ms(
            5,
            tick_callback,
            nullptr,
            &g_tick_timer
        )
    ) {
        secure_zero(
            g_draw_buffer.data(),
            g_draw_buffer.size() *
                sizeof(g_draw_buffer[0])
        );

        return false;
    }

    g_initialized = true;
    return true;
}

LvglPortFault lvgl_port_process() {
    if (!g_initialized) {
        return LvglPortFault::None;
    }

    lv_timer_handler();

    return g_runtime_fault;
}

bool lvgl_port_consume_touch_activity() {
    const bool activity =
        g_touch_activity;

    g_touch_activity = false;

    return activity;
}

void lvgl_port_arm_wake_guard() {
    g_wake_guard_armed = true;
}

void lvgl_port_cancel_wake_guard() {
    g_wake_guard_armed = false;
    g_suppress_until_release = false;
}

void lvgl_port_wipe_draw_buffer() {
    secure_zero(
        g_draw_buffer.data(),
        g_draw_buffer.size() *
            sizeof(g_draw_buffer[0])
    );
}

}  // namespace cryptomachine::ui
