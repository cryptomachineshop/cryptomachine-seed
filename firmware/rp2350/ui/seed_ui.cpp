#include "seed_ui.h"

#include "seed_app_controller.h"
#include "lvgl.h"

#include <cstddef>

namespace cryptomachine::ui {

namespace {

SeedAppController* g_app = nullptr;

constexpr lv_color_t kBackground =
    LV_COLOR_MAKE(0x11, 0x11, 0x11);

constexpr lv_color_t kOrange =
    LV_COLOR_MAKE(0xF7, 0x93, 0x1A);

constexpr lv_color_t kWhite =
    LV_COLOR_MAKE(0xF5, 0xF5, 0xF5);

constexpr lv_color_t kMuted =
    LV_COLOR_MAKE(0xA0, 0xA0, 0xA0);

constexpr lv_color_t kButtonDark =
    LV_COLOR_MAKE(0x24, 0x24, 0x24);

void prepare_screen() {
    lv_obj_t* screen = lv_scr_act();

    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(
        screen,
        kBackground,
        0
    );

    lv_obj_set_style_bg_opa(
        screen,
        LV_OPA_COVER,
        0
    );
}

lv_obj_t* make_label(
    const char* text,
    const lv_font_t* font,
    lv_color_t color
) {
    lv_obj_t* label =
        lv_label_create(lv_scr_act());

    lv_label_set_text(label, text);

    lv_obj_set_style_text_font(
        label,
        font,
        0
    );

    lv_obj_set_style_text_color(
        label,
        color,
        0
    );

    return label;
}

lv_obj_t* make_button(
    const char* text,
    lv_coord_t width,
    lv_coord_t height,
    lv_color_t background,
    lv_color_t text_color
) {
    lv_obj_t* button =
        lv_btn_create(lv_scr_act());

    lv_obj_set_size(
        button,
        width,
        height
    );

    lv_obj_set_style_bg_color(
        button,
        background,
        0
    );

    lv_obj_set_style_radius(
        button,
        14,
        0
    );

    lv_obj_t* label =
        lv_label_create(button);

    lv_label_set_text(label, text);

    lv_obj_set_style_text_font(
        label,
        &lv_font_montserrat_16,
        0
    );

    lv_obj_set_style_text_color(
        label,
        text_color,
        0
    );

    lv_obj_center(label);

    return button;
}

void create_header(
    const char* title,
    const char* subtitle
) {
    lv_obj_t* brand =
        make_label(
            "CryptoMachine",
            &lv_font_montserrat_24,
            kOrange
        );

    lv_obj_align(
        brand,
        LV_ALIGN_TOP_MID,
        0,
        28
    );

    lv_obj_t* seed =
        make_label(
            "SEED",
            &lv_font_montserrat_16,
            kWhite
        );

    lv_obj_align_to(
        seed,
        brand,
        LV_ALIGN_OUT_BOTTOM_MID,
        0,
        4
    );

    lv_obj_t* divider =
        lv_obj_create(lv_scr_act());

    lv_obj_set_size(
        divider,
        220,
        2
    );

    lv_obj_set_style_bg_color(
        divider,
        kOrange,
        0
    );

    lv_obj_set_style_bg_opa(
        divider,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        divider,
        0,
        0
    );

    lv_obj_align(
        divider,
        LV_ALIGN_TOP_MID,
        0,
        82
    );

    lv_obj_t* title_label =
        make_label(
            title,
            &lv_font_montserrat_24,
            kWhite
        );

    lv_obj_align(
        title_label,
        LV_ALIGN_TOP_MID,
        0,
        108
    );

    lv_obj_t* subtitle_label =
        make_label(
            subtitle,
            &lv_font_montserrat_14,
            kMuted
        );

    lv_obj_set_width(
        subtitle_label,
        270
    );

    lv_label_set_long_mode(
        subtitle_label,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_style_text_align(
        subtitle_label,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        subtitle_label,
        LV_ALIGN_TOP_MID,
        0,
        148
    );
}

void render_home();
void render_word_count();
void render_dice_intro();

void create_from_dice_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (g_app == nullptr) {
        return;
    }

    if (
        g_app->open_create_from_dice() ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void choose_12_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (g_app == nullptr) {
        return;
    }

    if (
        g_app->choose_dice_word_count(12) ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void choose_24_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (g_app == nullptr) {
        return;
    }

    if (
        g_app->choose_dice_word_count(24) ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void word_count_back_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (g_app == nullptr) {
        return;
    }

    if (
        g_app->dice_word_count_back() ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void render_home() {
    prepare_screen();

    create_header(
        "Generate Seed",
        "Create a BIP39 recovery phrase "
        "from physical dice."
    );

    lv_obj_t* button =
        make_button(
            "CREATE FROM DICE",
            250,
            72,
            kOrange,
            kBackground
        );

    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        45
    );

    lv_obj_add_event_cb(
        button,
        create_from_dice_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* footer =
        make_label(
            "Offline. No seed storage.",
            &lv_font_montserrat_14,
            kMuted
        );

    lv_obj_align(
        footer,
        LV_ALIGN_BOTTOM_MID,
        0,
        -28
    );
}

void render_word_count() {
    prepare_screen();

    create_header(
        "Seed Length",
        "Choose the BIP39 mnemonic length."
    );

    lv_obj_t* button_12 =
        make_button(
            "12 WORDS",
            250,
            70,
            kOrange,
            kBackground
        );

    lv_obj_align(
        button_12,
        LV_ALIGN_CENTER,
        0,
        -12
    );

    lv_obj_add_event_cb(
        button_12,
        choose_12_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* button_24 =
        make_button(
            "24 WORDS",
            250,
            70,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        button_24,
        LV_ALIGN_CENTER,
        0,
        76
    );

    lv_obj_add_event_cb(
        button_24,
        choose_24_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* back =
        make_button(
            "BACK",
            120,
            52,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        back,
        LV_ALIGN_BOTTOM_MID,
        0,
        -20
    );

    lv_obj_add_event_cb(
        back,
        word_count_back_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_dice_intro() {
    prepare_screen();

    create_header(
        "Dice Ceremony",
        "Your seed length is selected. "
        "Dice entry comes next."
    );

    const std::size_t words =
        g_app != nullptr
            ? g_app->word_count()
            : 0;

    const char* length_text =
        words == 24
            ? "24-WORD SEED"
            : "12-WORD SEED";

    lv_obj_t* selected =
        make_label(
            length_text,
            &lv_font_montserrat_24,
            kOrange
        );

    lv_obj_align(
        selected,
        LV_ALIGN_CENTER,
        0,
        20
    );

    lv_obj_t* explanation =
        make_label(
            "Five dice will be entered in "
            "fixed D1-D5 order.",
            &lv_font_montserrat_14,
            kWhite
        );

    lv_obj_set_width(
        explanation,
        260
    );

    lv_label_set_long_mode(
        explanation,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_style_text_align(
        explanation,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        explanation,
        LV_ALIGN_CENTER,
        0,
        85
    );

    lv_obj_t* footer =
        make_label(
            "Dice entry screen next",
            &lv_font_montserrat_14,
            kMuted
        );

    lv_obj_align(
        footer,
        LV_ALIGN_BOTTOM_MID,
        0,
        -28
    );
}

}  // namespace

void seed_ui_init(
    SeedAppController& app
) {
    g_app = &app;
    seed_ui_render();
}

void seed_ui_render() {
    if (g_app == nullptr) {
        return;
    }

    switch (g_app->state()) {
        case UIState::Home:
            render_home();
            break;

        case UIState::DiceWordCount:
            render_word_count();
            break;

        case UIState::DiceIntro:
            render_dice_intro();
            break;

        default:
            prepare_screen();

            create_header(
                "CryptoMachine Seed",
                "UI state not implemented yet."
            );

            break;
    }
}

}  // namespace cryptomachine::ui