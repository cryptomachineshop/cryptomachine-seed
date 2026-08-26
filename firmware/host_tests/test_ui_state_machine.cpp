#include "ui_state_machine.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_boot() {
    cryptomachine::UIStateMachine ui;

    check(
        ui.state() == cryptomachine::UIState::Boot,
        "new UI must start in Boot"
    );

    check(
        ui.boot_complete() ==
            cryptomachine::UIActionStatus::Success,
        "boot_complete must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "boot_complete must enter Home"
    );

    check(
        !ui.sensitive_data_present(),
        "boot must leave no sensitive data"
    );
}

void test_dice_navigation() {
    cryptomachine::UIStateMachine ui;

    check(
        ui.boot_complete() ==
            cryptomachine::UIActionStatus::Success,
        "dice navigation boot must succeed"
    );

    check(
        ui.open_create_from_dice() ==
            cryptomachine::UIActionStatus::Success,
        "open create-from-dice must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceWordCount,
        "dice workflow must enter word-count state"
    );

    check(
        ui.choose_dice_word_count(24) ==
            cryptomachine::UIActionStatus::Success,
        "24-word selection must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceIntro,
        "word-count selection must enter DiceIntro"
    );

    check(
        ui.word_count() == 24,
        "dice workflow must preserve word count"
    );

    check(
        ui.dice_session_active(),
        "dice session must be active"
    );

    check(
        ui.begin_dice_entry() ==
            cryptomachine::UIActionStatus::Success,
        "begin dice entry must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceEntry,
        "begin dice entry must enter DiceEntry"
    );

    check(
        ui.sensitive_data_present(),
        "dice entry must mark sensitive data present"
    );

    check(
        ui.dice_entry_to_review() ==
            cryptomachine::UIActionStatus::Success,
        "dice entry to review must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceShakeReview,
        "dice review state must be entered"
    );

    check(
        ui.shake_confirm_complete() ==
            cryptomachine::UIActionStatus::Success,
        "shake complete confirmation must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceComplete,
        "completed dice entry must enter DiceComplete"
    );

    check(
        ui.dice_complete_continue(false) ==
            cryptomachine::UIActionStatus::Success,
        "dice completion without warnings must continue"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceGenerateConfirm,
        "no-warning path must enter generate confirmation"
    );

    check(
        ui.generate_mnemonic() ==
            cryptomachine::UIActionStatus::Success,
        "mnemonic generation transition must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::MnemonicWordView,
        "generation must enter mnemonic word view"
    );

    check(
        ui.mnemonic_generated(),
        "generation state must mark mnemonic generated"
    );

    check(
        ui.mnemonic_words_complete() ==
            cryptomachine::UIActionStatus::Success,
        "mnemonic words complete must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::MnemonicFullReview,
        "mnemonic word view must advance to full review"
    );

    check(
        ui.mnemonic_review_finish() ==
            cryptomachine::UIActionStatus::Success,
        "mnemonic review finish must request destruction"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::SessionDestroyConfirm,
        "mnemonic finish must enter destroy confirmation"
    );

    check(
        ui.destroy_session() ==
            cryptomachine::UIActionStatus::Success,
        "destroy_session must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "destroy_session must return Home"
    );

    check(
        ui.word_count() == 0,
        "destroy_session must clear word count"
    );

    check(
        !ui.dice_session_active(),
        "destroy_session must clear dice session"
    );

    check(
        !ui.mnemonic_generated(),
        "destroy_session must clear mnemonic state"
    );

    check(
        !ui.sensitive_data_present(),
        "destroy_session must clear sensitive state"
    );
}

void test_destroy_cancel_go_back() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(12);
    ui.begin_dice_entry();

    check(
        ui.request_destroy() ==
            cryptomachine::UIActionStatus::Success,
        "destroy request must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::SessionDestroyConfirm,
        "destroy request must enter confirmation"
    );

    check(
        ui.destroy_go_back() ==
            cryptomachine::UIActionStatus::Success,
        "destroy cancel must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceEntry,
        "destroy cancel must return to DiceEntry"
    );

    check(
        ui.sensitive_data_present(),
        "destroy cancel must preserve sensitive state"
    );
}

void test_sanity_warning_continue() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();
    ui.dice_entry_to_review();
    ui.shake_confirm_complete();

    check(
        ui.dice_complete_continue(true) ==
            cryptomachine::UIActionStatus::Success,
        "warning path must continue"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceSanityWarning,
        "warning path must enter sanity warning"
    );

    check(
        ui.sanity_continue_anyway() ==
            cryptomachine::UIActionStatus::Success,
        "sanity continue must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceGenerateConfirm,
        "sanity continue must enter generate confirmation"
    );
}

void test_sanity_restart() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();
    ui.dice_entry_to_review();
    ui.shake_confirm_complete();
    ui.dice_complete_continue(true);

    check(
        ui.sanity_restart() ==
            cryptomachine::UIActionStatus::Success,
        "sanity restart must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceIntro,
        "sanity restart must return to DiceIntro"
    );

    check(
        ui.word_count() == 24,
        "sanity restart must preserve word count"
    );

    check(
        ui.dice_session_active(),
        "sanity restart must preserve active session mode"
    );

    check(
        !ui.mnemonic_generated(),
        "sanity restart must clear mnemonic state"
    );

    check(
        !ui.sensitive_data_present(),
        "sanity restart must clear old sensitive state"
    );
}

void test_final_word_flow() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();

    check(
        ui.open_final_word_tool() ==
            cryptomachine::UIActionStatus::Success,
        "final-word tool must open"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::FinalWordMode,
        "final-word tool must enter mode selection"
    );

    check(
        ui.choose_final_word_mode(24) ==
            cryptomachine::UIActionStatus::Success,
        "24-word final-word mode must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::FinalWordEntry,
        "final-word mode must enter word entry"
    );

    check(
        ui.expected_word_input() == 23,
        "24-word final-word mode must expect 23 words"
    );

    check(
        ui.sensitive_data_present(),
        "final-word entry must be sensitive"
    );

    check(
        ui.final_word_entry_complete() ==
            cryptomachine::UIActionStatus::Success,
        "final-word entry completion must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::FinalWordResults,
        "final-word completion must enter results"
    );

    check(
        ui.final_word_results_back() ==
            cryptomachine::UIActionStatus::Success,
        "final-word results back must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::FinalWordEntry,
        "results back must return to entry"
    );

    check(
        ui.expected_word_input() == 23,
        "results back must preserve expected input"
    );

    check(
        ui.sensitive_data_present(),
        "results back must preserve sensitive state"
    );

    ui.final_word_entry_complete();

    check(
        ui.final_word_results_home() ==
            cryptomachine::UIActionStatus::Success,
        "final-word results home must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "final-word results home must enter Home"
    );

    check(
        ui.expected_word_input() == 0,
        "final-word exit must clear expected input"
    );

    check(
        !ui.sensitive_data_present(),
        "final-word exit must clear sensitive state"
    );
}

void test_validation_flow() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();

    check(
        ui.open_validate_phrase() ==
            cryptomachine::UIActionStatus::Success,
        "validation tool must open"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseEntry,
        "validation tool must enter phrase entry"
    );

    check(
        ui.sensitive_data_present(),
        "validation entry must be sensitive"
    );

    check(
        ui.choose_validation_word_count(12) ==
            cryptomachine::UIActionStatus::Success,
        "12-word validation mode must succeed"
    );

    check(
        ui.expected_word_input() == 12,
        "validation must expect 12 words"
    );

    check(
        ui.validation_complete() ==
            cryptomachine::UIActionStatus::Success,
        "validation completion must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseResult,
        "validation completion must enter result"
    );

    check(
        ui.validation_review_words() ==
            cryptomachine::UIActionStatus::Success,
        "validation review must return to words"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseEntry,
        "validation review must return to entry"
    );

    check(
        ui.expected_word_input() == 12,
        "validation review must preserve selected count"
    );

    ui.validation_complete();

    check(
        ui.validation_start_over() ==
            cryptomachine::UIActionStatus::Success,
        "validation start-over must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseEntry,
        "validation start-over must remain in entry"
    );

    check(
        ui.expected_word_input() == 0,
        "validation start-over must clear word count"
    );

    check(
        !ui.sensitive_data_present(),
        "validation start-over must clear sensitive state"
    );

    ui.choose_validation_word_count(24);
    ui.validation_complete();

    check(
        ui.validation_result_home() ==
            cryptomachine::UIActionStatus::Success,
        "validation result home must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "validation result home must enter Home"
    );

    check(
        ui.expected_word_input() == 0,
        "validation exit must clear expected input"
    );

    check(
        !ui.sensitive_data_present(),
        "validation exit must clear sensitive state"
    );
}

void test_invalid_transition_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();

    check(
        ui.generate_mnemonic() ==
            cryptomachine::UIActionStatus::InvalidState,
        "generation from Home must be rejected"
    );
}

void test_invalid_word_counts_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();

    check(
        ui.choose_dice_word_count(18) ==
            cryptomachine::UIActionStatus::InvalidValue,
        "unsupported dice word count must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceWordCount,
        "invalid word count must not change UI state"
    );
}

}  // namespace

int main() {
    test_boot();
    test_dice_navigation();
    test_destroy_cancel_go_back();
    test_sanity_warning_continue();
    test_sanity_restart();
    test_final_word_flow();
    test_validation_flow();
    test_invalid_transition_rejected();
    test_invalid_word_counts_rejected();

    if (failures != 0) {
        std::cerr
            << failures
            << " UI state-machine test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ UI state-machine tests passed.\n";

    return 0;
}