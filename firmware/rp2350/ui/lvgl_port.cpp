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
        !cryptomachine::hardware::display_set_window(
            x_start,
            y_start,
            x_end,
            y_end
        )
    ) {
        // LVGL may already have rendered sensitive pixels
        // into the draw buffer even if the display transfer
        // cannot proceed.
        wipe_pixel_buffer(
            color_buffer,
            pixel_count
        );

        lv_disp_flush_ready(driver);
        return;
    }

    // This write is blocking. When it returns, the ST7796
    // has consumed the bytes and the RAM copy is no longer
    // needed.
    cryptomachine::hardware::display_write_bytes(
        reinterpret_cast<const std::uint8_t*>(
            color_buffer
        ),
        pixel_count * sizeof(lv_color_t)
    );

    // Mnemonic glyphs can exist here as raw RGB565 pixels.
    // Scrub the flushed region before returning ownership of
    // the buffer to LVGL.
    wipe_pixel_buffer(
        color_buffer,
        pixel_count
    );

    lv_disp_flush_ready(driver);
}

void touch_read_callback(
    lv_indev_drv_t*,
    lv_indev_data_t* data
) {
    if (data == nullptr) {
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

        data->point.x =
            static_cast<lv_coord_t>(
                point.x
            );

        data->point.y =
            static_cast<lv_coord_t>(
                point.y
            );

        data->state =
            LV_INDEV_STATE_PRESSED;

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

    data->state =
        LV_INDEV_STATE_RELEASED;
}

}  // namespace

bool lvgl_port_init() {
    if (g_initialized) {
        return true;
    }

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

void lvgl_port_process() {
    if (!g_initialized) {
        return;
    }

    lv_timer_handler();
}

void lvgl_port_wipe_draw_buffer() {
    secure_zero(
        g_draw_buffer.data(),
        g_draw_buffer.size() *
            sizeof(g_draw_buffer[0])
    );
}

}  // namespace cryptomachine::ui
