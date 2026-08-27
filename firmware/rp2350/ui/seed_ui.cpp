#include "seed_ui.h"

#include "bip39_wordlist.h"
#include "lvgl_port.h"
#include "seed_app_controller.h"
#include "secure_zero.h"
#include "lvgl.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace cryptomachine::ui {

namespace {

SeedAppController* g_app = nullptr;

SeedUiFault g_ui_fault =
    SeedUiFault::None;

std::array<char, kDiceCount> g_dice_entry{};
std::size_t g_dice_entry_count = 0;

std::array<lv_obj_t*, kWordCount24> g_sensitive_word_labels{};
std::size_t g_sensitive_word_label_count = 0;

std::size_t g_mnemonic_word_index = 0;

constexpr std::array<const char*, kWordCount24>
    kWordNumbers = {
        "1", "2", "3", "4", "5", "6",
        "7", "8", "9", "10", "11", "12",
        "13", "14", "15", "16", "17", "18",
        "19", "20", "21", "22", "23", "24"
    };

constexpr std::array<const char*, kWordCount12>
    kWordPositions12 = {
        "WORD 1 OF 12", "WORD 2 OF 12", "WORD 3 OF 12",
        "WORD 4 OF 12", "WORD 5 OF 12", "WORD 6 OF 12",
        "WORD 7 OF 12", "WORD 8 OF 12", "WORD 9 OF 12",
        "WORD 10 OF 12", "WORD 11 OF 12", "WORD 12 OF 12"
    };

constexpr std::array<const char*, kWordCount24>
    kWordPositions24 = {
        "WORD 1 OF 24", "WORD 2 OF 24", "WORD 3 OF 24",
        "WORD 4 OF 24", "WORD 5 OF 24", "WORD 6 OF 24",
        "WORD 7 OF 24", "WORD 8 OF 24", "WORD 9 OF 24",
        "WORD 10 OF 24", "WORD 11 OF 24", "WORD 12 OF 24",
        "WORD 13 OF 24", "WORD 14 OF 24", "WORD 15 OF 24",
        "WORD 16 OF 24", "WORD 17 OF 24", "WORD 18 OF 24",
        "WORD 19 OF 24", "WORD 20 OF 24", "WORD 21 OF 24",
        "WORD 22 OF 24", "WORD 23 OF 24", "WORD 24 OF 24"
    };

void wipe_dice_entry() {
    secure_zero(
        g_dice_entry.data(),
        g_dice_entry.size()
    );

    g_dice_entry_count = 0;
}

void clear_sensitive_word_label_refs() {
    for (
        std::size_t i = 0;
        i < g_sensitive_word_label_count;
        ++i
    ) {
        if (g_sensitive_word_labels[i] != nullptr) {
            // The BIP39 dictionary itself is public, but the
            // selected pointer identifies a secret mnemonic word.
            // Replace the pointer before LVGL deletes the object.
            lv_label_set_text_static(
                g_sensitive_word_labels[i],
                ""
            );

            g_sensitive_word_labels[i] = nullptr;
        }
    }

    g_sensitive_word_label_count = 0;
}

void reset_sensitive_ui_state() {
    clear_sensitive_word_label_refs();
    wipe_dice_entry();
    g_mnemonic_word_index = 0;
}


void latch_ui_fault(
    SeedUiFault fault
) {
    if (
        g_ui_fault ==
        SeedUiFault::None
    ) {
        g_ui_fault = fault;
    }
}

SeedAppController* app_or_fault() {
    if (g_app == nullptr) {
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );

        return nullptr;
    }

    return g_app;
}

bool controller_action_succeeded(
    SeedAppStatus status
) {
    if (status == SeedAppStatus::Success) {
        return true;
    }

    latch_ui_fault(
        SeedUiFault::ControllerActionError
    );

    return false;
}

void render_after_controller_action(
    SeedAppStatus status
) {
    if (controller_action_succeeded(status)) {
        seed_ui_render();
    }
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

    clear_sensitive_word_label_refs();

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

lv_obj_t* make_sensitive_word_label(
    std::uint16_t word_index,
    const lv_font_t* font,
    lv_color_t color,
    lv_coord_t width,
    lv_coord_t height
) {
    if (
        word_index >=
        kBip39EnglishWordlist.size()
    ) {
        return nullptr;
    }

    if (
        g_sensitive_word_label_count >=
        g_sensitive_word_labels.size()
    ) {
        return nullptr;
    }

    lv_obj_t* label =
        lv_label_create(lv_scr_act());

    // Generated entries are string_views backed by string
    // literals. Static text avoids copying the secret word
    // into LVGL's heap.
    lv_label_set_text_static(
        label,
        kBip39EnglishWordlist[word_index].data()
    );

    lv_obj_set_size(
        label,
        width,
        height
    );

    lv_label_set_long_mode(
        label,
        LV_LABEL_LONG_CLIP
    );

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

    g_sensitive_word_labels[
        g_sensitive_word_label_count
    ] = label;

    ++g_sensitive_word_label_count;

    return label;
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
void render_dice_entry();
void render_dice_shake_review();
void render_dice_complete();
void render_dice_sanity_warning();
void render_dice_generate_confirm();
void render_mnemonic_word_view();
void render_mnemonic_full_review();
void render_session_destroy_confirm();

void begin_dice_entry_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->begin_dice_entry()
    );
}

void create_from_dice_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->open_create_from_dice()
    );
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->choose_dice_word_count(12)
    );
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->choose_dice_word_count(24)
    );
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->dice_word_count_back()
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    if (g_dice_entry_count >= kDiceCount) {
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );
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
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );
        return;
    }

    g_dice_entry[g_dice_entry_count] =
        face[0];

    ++g_dice_entry_count;

    if (g_dice_entry_count == kDiceCount) {
        const SeedAppStatus status =
            app->enter_shake_for_review(
                std::string_view(
                    g_dice_entry.data(),
                    g_dice_entry.size()
                )
            );

        wipe_dice_entry();

        if (
            controller_action_succeeded(
                status
            )
        ) {
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    wipe_dice_entry();

    render_after_controller_action(
        app->confirm_pending_shake()
    );
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

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    wipe_dice_entry();

    render_after_controller_action(
        app->reenter_pending_shake()
    );
}

void dice_complete_continue_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->continue_from_dice_complete()
    );
}

void sanity_continue_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->sanity_continue_anyway()
    );
}

void sanity_restart_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    wipe_dice_entry();

    render_after_controller_action(
        app->sanity_restart()
    );
}

void generate_back_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->generate_back()
    );
}

void generate_mnemonic_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    if (
        controller_action_succeeded(
            app->generate_mnemonic()
        )
    ) {
        g_mnemonic_word_index = 0;
        seed_ui_render();
    }
}

void mnemonic_previous_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    if (g_mnemonic_word_index == 0) {
        return;
    }

    --g_mnemonic_word_index;
    seed_ui_render();
}

void mnemonic_next_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    const SeedResult* result =
        app->seed_result();

    if (
        result == nullptr ||
        (
            result->mnemonic.word_count !=
                kWordCount12 &&
            result->mnemonic.word_count !=
                kWordCount24
        )
    ) {
        latch_ui_fault(
            SeedUiFault::InvalidSeedResult
        );
        return;
    }

    if (
        g_mnemonic_word_index >=
        result->mnemonic.word_count
    ) {
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );
        return;
    }

    if (
        g_mnemonic_word_index + 1 <
        result->mnemonic.word_count
    ) {
        ++g_mnemonic_word_index;
        seed_ui_render();
        return;
    }

    render_after_controller_action(
        app->mnemonic_words_complete()
    );
}

void mnemonic_review_finish_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->mnemonic_review_finish()
    );
}

void request_destroy_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->request_destroy()
    );
}

void destroy_go_back_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    render_after_controller_action(
        app->destroy_go_back()
    );
}

void destroy_session_event(
    lv_event_t* event
) {
    if (
        lv_event_get_code(event) !=
        LV_EVENT_CLICKED
    ) {
        return;
    }

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    clear_sensitive_word_label_refs();
    wipe_dice_entry();
    g_mnemonic_word_index = 0;

    lvgl_port_wipe_draw_buffer();

    render_after_controller_action(
        app->destroy_session()
    );
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

    if (
        words != kWordCount12 &&
        words != kWordCount24
    ) {
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );
        return;
    }

    const char* length_text =
        words == kWordCount24
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

void render_dice_complete() {
    prepare_screen();

    create_header(
        "Dice Complete",
        "All required physical dice "
        "outcomes have been captured."
    );

    SeedAppController* app =
        app_or_fault();

    if (app == nullptr) {
        return;
    }

    const std::size_t words =
        app->word_count();

    if (
        words != kWordCount12 &&
        words != kWordCount24
    ) {
        latch_ui_fault(
            SeedUiFault::UnexpectedApplicationState
        );
        return;
    }

    const char* ceremony_text =
        words == kWordCount24
            ? "20 SHAKES COMPLETE"
            : "10 SHAKES COMPLETE";

    lv_obj_t* complete =
        make_label(
            ceremony_text,
            &lv_font_montserrat_24,
            kOrange
        );

    lv_obj_align(
        complete,
        LV_ALIGN_CENTER,
        0,
        15
    );

    lv_obj_t* message =
        make_label(
            "Next: dice sanity checks "
            "before seed generation.",
            &lv_font_montserrat_14,
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
        75
    );

    lv_obj_t* button =
        make_button(
            "CONTINUE",
            250,
            70,
            kOrange,
            kBackground
        );

    lv_obj_align(
        button,
        LV_ALIGN_BOTTOM_MID,
        0,
        -38
    );

    lv_obj_add_event_cb(
        button,
        dice_complete_continue_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_dice_sanity_warning() {
    prepare_screen();

    create_header(
        "Dice Warning",
        "The entered dice show an unusual "
        "pattern."
    );

    lv_obj_t* warning =
        make_label(
            "This does not prove the dice are "
            "invalid, but the pattern deserves "
            "attention before generating a seed.",
            &lv_font_montserrat_14,
            kWhite
        );

    lv_obj_set_width(
        warning,
        270
    );

    lv_label_set_long_mode(
        warning,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_style_text_align(
        warning,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        warning,
        LV_ALIGN_CENTER,
        0,
        -25
    );

    lv_obj_t* continue_button =
        make_button(
            "CONTINUE ANYWAY",
            250,
            68,
            kOrange,
            kBackground
        );

    lv_obj_align(
        continue_button,
        LV_ALIGN_CENTER,
        0,
        82
    );

    lv_obj_add_event_cb(
        continue_button,
        sanity_continue_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* restart_button =
        make_button(
            "RESTART DICE",
            210,
            54,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        restart_button,
        LV_ALIGN_BOTTOM_MID,
        0,
        -24
    );

    lv_obj_add_event_cb(
        restart_button,
        sanity_restart_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_dice_generate_confirm() {
    prepare_screen();

    create_header(
        "Generate Seed",
        "Dice collection and sanity checks "
        "are complete."
    );

    lv_obj_t* ready =
        make_label(
            "READY TO GENERATE",
            &lv_font_montserrat_24,
            kOrange
        );

    lv_obj_align(
        ready,
        LV_ALIGN_CENTER,
        0,
        -5
    );

    lv_obj_t* message =
        make_label(
            "The next screen reveals your "
            "BIP39 mnemonic. Record it privately.",
            &lv_font_montserrat_14,
            kWhite
        );

    lv_obj_set_width(
        message,
        270
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
        55
    );

    lv_obj_t* generate =
        make_button(
            "GENERATE SEED",
            250,
            70,
            kOrange,
            kBackground
        );

    lv_obj_align(
        generate,
        LV_ALIGN_CENTER,
        0,
        135
    );

    lv_obj_add_event_cb(
        generate,
        generate_mnemonic_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* back =
        make_button(
            "BACK",
            130,
            50,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        back,
        LV_ALIGN_BOTTOM_MID,
        0,
        -14
    );

    lv_obj_add_event_cb(
        back,
        generate_back_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_mnemonic_word_view() {
    prepare_screen();

    const SeedResult* result =
        g_app != nullptr
            ? g_app->seed_result()
            : nullptr;

    if (
        result == nullptr ||
        (
            result->mnemonic.word_count !=
                kWordCount12 &&
            result->mnemonic.word_count !=
                kWordCount24
        ) ||
        g_mnemonic_word_index >=
            result->mnemonic.word_count
    ) {
        latch_ui_fault(
            SeedUiFault::InvalidSeedResult
        );
        return;
    }

    create_header(
        "Record Seed",
        "Write each word in exact order."
    );

    const std::size_t word_count =
        result->mnemonic.word_count;

    const char* position_text =
        word_count == kWordCount24
            ? kWordPositions24[
                g_mnemonic_word_index
            ]
            : kWordPositions12[
                g_mnemonic_word_index
            ];

    lv_obj_t* position =
        make_label(
            position_text,
            &lv_font_montserrat_16,
            kMuted
        );

    lv_obj_align(
        position,
        LV_ALIGN_TOP_MID,
        0,
        205
    );

    const std::uint16_t bip39_index =
        result->mnemonic.word_indices[
            g_mnemonic_word_index
        ];

    lv_obj_t* word =
        make_sensitive_word_label(
            bip39_index,
            &lv_font_montserrat_24,
            kOrange,
            280,
            52
        );

    if (word == nullptr) {
        latch_ui_fault(
            SeedUiFault::InvalidSeedResult
        );
        return;
    }

    lv_obj_set_style_text_align(
        word,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        word,
        LV_ALIGN_TOP_MID,
        0,
        250
    );

    if (g_mnemonic_word_index > 0) {
        lv_obj_t* previous =
            make_button(
                "PREVIOUS",
                125,
                54,
                kButtonDark,
                kWhite
            );

        lv_obj_align(
            previous,
            LV_ALIGN_CENTER,
            -70,
            100
        );

        lv_obj_add_event_cb(
            previous,
            mnemonic_previous_event,
            LV_EVENT_CLICKED,
            nullptr
        );
    }

    const bool last_word =
        g_mnemonic_word_index + 1 ==
        word_count;

    lv_obj_t* next =
        make_button(
            last_word
                ? "FULL REVIEW"
                : "NEXT",
            125,
            54,
            kOrange,
            kBackground
        );

    lv_obj_align(
        next,
        LV_ALIGN_CENTER,
        70,
        100
    );

    lv_obj_add_event_cb(
        next,
        mnemonic_next_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* destroy =
        make_button(
            "DESTROY SESSION",
            190,
            46,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        destroy,
        LV_ALIGN_BOTTOM_MID,
        0,
        -12
    );

    lv_obj_add_event_cb(
        destroy,
        request_destroy_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_mnemonic_full_review() {
    prepare_screen();

    const SeedResult* result =
        g_app != nullptr
            ? g_app->seed_result()
            : nullptr;

    if (
        result == nullptr ||
        (
            result->mnemonic.word_count !=
                kWordCount12 &&
            result->mnemonic.word_count !=
                kWordCount24
        )
    ) {
        latch_ui_fault(
            SeedUiFault::InvalidSeedResult
        );
        return;
    }

    create_header(
        "Full Review",
        "Verify every word and exact order."
    );

    const std::size_t word_count =
        result->mnemonic.word_count;

    const std::size_t rows =
        word_count == kWordCount24
            ? 12
            : 6;

    constexpr lv_coord_t kStartY = 178;
    constexpr lv_coord_t kRowStep = 20;

    for (
        std::size_t i = 0;
        i < word_count;
        ++i
    ) {
        const std::size_t column =
            i / rows;

        const std::size_t row =
            i % rows;

        const lv_coord_t number_x =
            column == 0
                ? 8
                : 163;

        const lv_coord_t word_x =
            column == 0
                ? 30
                : 188;

        const lv_coord_t y =
            static_cast<lv_coord_t>(
                kStartY +
                row * kRowStep
            );

        lv_obj_t* number =
            make_label(
                kWordNumbers[i],
                &lv_font_montserrat_14,
                kMuted
            );

        lv_obj_align(
            number,
            LV_ALIGN_TOP_LEFT,
            number_x,
            y
        );

        const std::uint16_t bip39_index =
            result->mnemonic.word_indices[i];

        lv_obj_t* word =
            make_sensitive_word_label(
                bip39_index,
                &lv_font_montserrat_14,
                kWhite,
                122,
                18
            );

        if (word == nullptr) {
            latch_ui_fault(
                SeedUiFault::InvalidSeedResult
            );
            return;
        }

        lv_obj_align(
            word,
            LV_ALIGN_TOP_LEFT,
            word_x,
            y
        );
    }

    lv_obj_t* finish =
        make_button(
            "FINISH & DESTROY",
            220,
            50,
            kOrange,
            kBackground
        );

    lv_obj_align(
        finish,
        LV_ALIGN_BOTTOM_MID,
        0,
        -8
    );

    lv_obj_add_event_cb(
        finish,
        mnemonic_review_finish_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}

void render_session_destroy_confirm() {
    prepare_screen();

    create_header(
        "Destroy Session",
        "This action cannot be undone."
    );

    lv_obj_t* warning =
        make_label(
            "Wipe the generated mnemonic, "
            "dice history, sanity data, and "
            "session state from RAM?",
            &lv_font_montserrat_16,
            kWhite
        );

    lv_obj_set_width(
        warning,
        270
    );

    lv_label_set_long_mode(
        warning,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_style_text_align(
        warning,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        warning,
        LV_ALIGN_CENTER,
        0,
        -25
    );

    lv_obj_t* destroy =
        make_button(
            "DESTROY & RETURN HOME",
            270,
            68,
            kOrange,
            kBackground
        );

    lv_obj_align(
        destroy,
        LV_ALIGN_CENTER,
        0,
        82
    );

    lv_obj_add_event_cb(
        destroy,
        destroy_session_event,
        LV_EVENT_CLICKED,
        nullptr
    );

    lv_obj_t* back =
        make_button(
            "GO BACK",
            160,
            50,
            kButtonDark,
            kWhite
        );

    lv_obj_align(
        back,
        LV_ALIGN_BOTTOM_MID,
        0,
        -18
    );

    lv_obj_add_event_cb(
        back,
        destroy_go_back_event,
        LV_EVENT_CLICKED,
        nullptr
    );
}
}  // namespace

void seed_ui_emergency_clear() {
    clear_sensitive_word_label_refs();
    wipe_dice_entry();
    g_mnemonic_word_index = 0;

    lv_obj_t* screen = lv_scr_act();

    if (screen != nullptr) {
        lv_obj_clean(screen);
    }
}

void seed_ui_init(
    SeedAppController& app
) {
    reset_sensitive_ui_state();

    g_ui_fault =
        SeedUiFault::None;

    g_app = &app;
    seed_ui_render();
}

SeedUiFault seed_ui_fault() {
    return g_ui_fault;
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

        case UIState::DiceComplete:
            render_dice_complete();
            break;

        case UIState::DiceSanityWarning:
            render_dice_sanity_warning();
            break;

        case UIState::DiceGenerateConfirm:
            render_dice_generate_confirm();
            break;

        case UIState::MnemonicWordView:
            render_mnemonic_word_view();
            break;

        case UIState::MnemonicFullReview:
            render_mnemonic_full_review();
            break;

        case UIState::SessionDestroyConfirm:
            render_session_destroy_confirm();
            break;

        default:
            prepare_screen();

            latch_ui_fault(
                SeedUiFault::UnexpectedApplicationState
            );

            break;
    }
}

}  // namespace cryptomachine::ui
