#include "bringup_screen.h"

#include "lvgl.h"

#include <cstdint>

namespace cryptomachine::ui {
namespace {

lv_obj_t* g_status_label = nullptr;
std::uint32_t g_touch_count = 0;

void test_button_event(lv_event_t* event) {
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    ++g_touch_count;

    if (g_status_label != nullptr) {
        lv_label_set_text_fmt(
            g_status_label,
            "Touch OK  |  %lu",
            static_cast<unsigned long>(g_touch_count)
        );
    }
}

}  // namespace

void create_bringup_screen() {
    lv_obj_t* screen = lv_scr_act();

    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(0x111111),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        screen,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_t* title = lv_label_create(screen);

    lv_label_set_text(
        title,
        "CryptoMachine"
    );

    lv_obj_set_style_text_color(
        title,
        lv_color_hex(0xF7931A),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        title,
        &lv_font_montserrat_24,
        LV_PART_MAIN
    );

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        45
    );

    lv_obj_t* subtitle = lv_label_create(screen);

    lv_label_set_text(
        subtitle,
        "SEED"
    );

    lv_obj_set_style_text_color(
        subtitle,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        subtitle,
        &lv_font_montserrat_16,
        LV_PART_MAIN
    );

    lv_obj_align_to(
        subtitle,
        title,
        LV_ALIGN_OUT_BOTTOM_MID,
        0,
        8
    );

    lv_obj_t* divider = lv_obj_create(screen);

    lv_obj_remove_style_all(divider);

    lv_obj_set_size(
        divider,
        220,
        2
    );

    lv_obj_set_style_bg_color(
        divider,
        lv_color_hex(0xF7931A),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        divider,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_align(
        divider,
        LV_ALIGN_TOP_MID,
        0,
        115
    );

    lv_obj_t* hardware_label = lv_label_create(screen);

    lv_label_set_text(
        hardware_label,
        "RP2350B\nST7796 DISPLAY\nFT6336U TOUCH"
    );

    lv_obj_set_style_text_color(
        hardware_label,
        lv_color_hex(0xD0D0D0),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        hardware_label,
        &lv_font_montserrat_14,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        hardware_label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        hardware_label,
        LV_ALIGN_TOP_MID,
        0,
        145
    );

    lv_obj_t* button = lv_btn_create(screen);

    lv_obj_set_size(
        button,
        220,
        80
    );

    lv_obj_set_style_bg_color(
        button,
        lv_color_hex(0xF7931A),
        LV_PART_MAIN
    );

    lv_obj_set_style_radius(
        button,
        14,
        LV_PART_MAIN
    );

    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        55
    );

    lv_obj_add_event_cb(
        button,
        test_button_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* button_label = lv_label_create(button);

    lv_label_set_text(
        button_label,
        "TEST TOUCH"
    );

    lv_obj_set_style_text_color(
        button_label,
        lv_color_hex(0x111111),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        button_label,
        &lv_font_montserrat_16,
        LV_PART_MAIN
    );

    lv_obj_center(button_label);

    g_status_label = lv_label_create(screen);

    lv_label_set_text(
        g_status_label,
        "Waiting for touch..."
    );

    lv_obj_set_style_text_color(
        g_status_label,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        g_status_label,
        &lv_font_montserrat_14,
        LV_PART_MAIN
    );

    lv_obj_align(
        g_status_label,
        LV_ALIGN_BOTTOM_MID,
        0,
        -55
    );

    lv_obj_t* footer = lv_label_create(screen);

    lv_label_set_text(
        footer,
        "Hardware bring-up"
    );

    lv_obj_set_style_text_color(
        footer,
        lv_color_hex(0x777777),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_font(
        footer,
        &lv_font_montserrat_14,
        LV_PART_MAIN
    );

    lv_obj_align(
        footer,
        LV_ALIGN_BOTTOM_MID,
        0,
        -20
    );
}

}  // namespace cryptomachine::ui