#include "seed_ui.h"

#include "seed_app_controller.h"
#include "secure_zero.h"
#include "lvgl.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace cryptomachine::ui {

namespace {

SeedAppController* g_app = nullptr;

std::array<char, kDiceCount> g_dice_entry{};
std::size_t g_dice_entry_count = 0;

void wipe_dice_entry() {
    secure_zero(
        g_dice_entry.data(),
        g_dice_entry.size()
    );

    g_dice_entry_count = 0;
}

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

void begin_dice_entry_event(
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
        g_app->begin_dice_entry() ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void render_home();
void render_word_count();
void render_dice_intro();
void render_dice_entry();
void render_dice_shake_review();

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

    lv_obj_t* begin =
        make_button(
            "BEGIN DICE ENTRY",
            250,
            70,
            kOrange,
            kBackground
        );

    lv_obj_align(
        begin,
        LV_ALIGN_CENTER,
        0,
        145
    );

    lv_obj_add_event_cb(
        begin,
        begin_dice_entry_event,
        LV_EVENT_CLICKED,
        nullptr
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

void dice_face_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (
        g_app == nullptr ||
        g_dice_entry_count >= kDiceCount
    ) {
        return;
    }

    const char* face =
        static_cast<const char*>(
            lv_event_get_user_data(event)
        );

    if (
        face == nullptr ||
        face[0] < '1' ||
        face[0] > '6'
    ) {
        return;
    }

    g_dice_entry[g_dice_entry_count] =
        face[0];

    ++g_dice_entry_count;

    if (g_dice_entry_count == kDiceCount) {
        const SeedAppStatus status =
            g_app->enter_shake_for_review(
                std::string_view(
                    g_dice_entry.data(),
                    g_dice_entry.size()
                )
            );

        // The controller now owns its fixed copy.
        // Wipe the UI-side entry buffer immediately.
        wipe_dice_entry();

        if (status == SeedAppStatus::Success) {
            seed_ui_render();
        }

        return;
    }

    seed_ui_render();
}

void confirm_shake_event(
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

    wipe_dice_entry();

    if (
        g_app->confirm_pending_shake() ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void reenter_shake_event(
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

    wipe_dice_entry();

    if (
        g_app->reenter_pending_shake() ==
        SeedAppStatus::Success
    ) {
        seed_ui_render();
    }
}

void render_dice_entry() {
    prepare_screen();

    create_header(
        "Dice Entry",
        "Roll five dice and enter them "
        "in fixed D1-D5 order."
    );

    static const char* const prompts[] = {
        "Enter D1",
        "Enter D2",
        "Enter D3",
        "Enter D4",
        "Enter D5",
        "Five dice entered"
    };

    static const char* const progress[] = {
        "0 / 5 entered",
        "1 / 5 entered",
        "2 / 5 entered",
        "3 / 5 entered",
        "4 / 5 entered",
        "5 / 5 entered"
    };

    const std::size_t index =
        g_dice_entry_count <= kDiceCount
            ? g_dice_entry_count
            : kDiceCount;

    lv_obj_t* prompt =
        make_label(
            prompts[index],
            &lv_font_montserrat_24,
            kOrange
        );

    lv_obj_align(
        prompt,
        LV_ALIGN_TOP_MID,
        0,
        205
    );

    lv_obj_t* progress_label =
        make_label(
            progress[index],
            &lv_font_montserrat_14,
            kMuted
        );

    lv_obj_align(
        progress_label,
        LV_ALIGN_TOP_MID,
        0,
        242
    );

    if (g_dice_entry_count < kDiceCount) {
        static const char* const faces[] = {
            "1", "2", "3", "4", "5", "6"
        };

        for (std::size_t i = 0; i < 6; ++i) {
            lv_obj_t* button =
                make_button(
                    faces[i],
                    78,
                    62,
                    kButtonDark,
                    kWhite
                );

            const lv_coord_t x =
                static_cast<lv_coord_t>(
                    (i % 3) * 88 - 88
                );

            const lv_coord_t y =
                static_cast<lv_coord_t>(
                    (i / 3) * 74 + 305
                );

            lv_obj_align(
                button,
                LV_ALIGN_TOP_MID,
                x,
                y
            );

            lv_obj_add_event_cb(
                button,
                dice_face_event,
                LV_EVENT_CLICKED,
                const_cast<char*>(faces[i])
            );
        }
    } else {
        lv_obj_t* ready =
            make_label(
                "Ready for shake review",
                &lv_font_montserrat_16,
                kWhite
            );

        lv_obj_align(
            ready,
            LV_ALIGN_CENTER,
            0,
            140
        );
    }
}

void render_dice_shake_review() {
    prepare_screen();

    create_header(
        "Review Shake",
        "Five dice captured securely."
    );

    lv_obj_t* message =
        make_label(
            "Confirm this shake or "
            "re-enter all five dice.",
            &lv_font_montserrat_16,
            kWhite
        );

    lv_obj_set_width(
        message,
        260
    );

    lv_label_set_long_mode(
        message,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_style_text_align(
        message,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        message,
        LV_ALIGN_CENTER,
        0,
        -10
    );

    lv_obj_t* confirm =
        make_button(
            "CONFIRM SHAKE",
            250,
            70,
            kOrange,
            kBackground
        );

    lv_obj_align(
        confirm,
        LV_ALIGN_CENTER,
        0,
        80
    );

    lv_obj_add_event_cb(
        confirm,
        confirm_shake_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* reenter =
        make_button(
            "RE-ENTER",
            180,
            54,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        reenter,
        LV_ALIGN_BOTTOM_MID,
        0,
        -24
    );

    lv_obj_add_event_cb(
        reenter,
        reenter_shake_event,
        LV_EVENT_CLICKED,
        nullptr
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

        case UIState::DiceEntry:
            render_dice_entry();
            break;

        case UIState::DiceShakeReview:
            render_dice_shake_review();
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