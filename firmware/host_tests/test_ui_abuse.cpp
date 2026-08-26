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

void test_generate_from_home_rejected() {
    cryptomachine::UIStateMachine ui;
    ui.boot_complete();

    check(
        ui.generate_mnemonic() ==
            cryptomachine::UIActionStatus::InvalidState,
        "generation from Home must be rejected"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "rejected generation must remain at Home"
    );

    check(
        !ui.mnemonic_generated(),
        "rejected generation must not mark mnemonic generated"
    );
}

void test_begin_dice_without_session_rejected() {
    cryptomachine::UIStateMachine ui;
    ui.boot_complete();

    check(
        ui.begin_dice_entry() ==
            cryptomachine::UIActionStatus::InvalidState,
        "dice entry from Home must be rejected"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "rejected dice entry must remain at Home"
    );
}

void test_invalid_dice_word_count_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();

    check(
        ui.choose_dice_word_count(18) ==
            cryptomachine::UIActionStatus::InvalidValue,
        "invalid dice word count must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceWordCount,
        "invalid dice word count must preserve state"
    );

    check(
        !ui.dice_session_active(),
        "invalid dice word count must not activate session"
    );
}

void test_generate_too_early_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();

    check(
        ui.generate_mnemonic() ==
            cryptomachine::UIActionStatus::InvalidState,
        "early mnemonic generation must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceEntry,
        "early generation rejection must preserve DiceEntry"
    );

    check(
        !ui.mnemonic_generated(),
        "early generation must not mark mnemonic generated"
    );
}

void test_destroy_from_home_rejected() {
    cryptomachine::UIStateMachine ui;
    ui.boot_complete();

    check(
        ui.request_destroy() ==
            cryptomachine::UIActionStatus::NotSensitiveWorkflow,
        "destroy request from Home must be rejected"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "rejected destroy request must preserve Home"
    );
}

void test_double_destroy_request_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(12);
    ui.begin_dice_entry();

    check(
        ui.request_destroy() ==
            cryptomachine::UIActionStatus::Success,
        "first destroy request must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::SessionDestroyConfirm,
        "first destroy request must enter confirmation"
    );

    check(
        ui.request_destroy() ==
            cryptomachine::UIActionStatus::
                AlreadyInDestroyConfirmation,
        "second destroy request must be rejected"
    );
}

void test_destroy_go_back_preserves_session() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(12);
    ui.begin_dice_entry();

    check(
        ui.sensitive_data_present(),
        "dice entry must be sensitive before destroy request"
    );

    ui.request_destroy();

    check(
        ui.destroy_go_back() ==
            cryptomachine::UIActionStatus::Success,
        "destroy go-back must succeed"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceEntry,
        "destroy go-back must return to DiceEntry"
    );

    check(
        ui.word_count() == 12,
        "destroy go-back must preserve word count"
    );

    check(
        ui.dice_session_active(),
        "destroy go-back must preserve dice session"
    );

    check(
        ui.sensitive_data_present(),
        "destroy go-back must preserve sensitive state"
    );
}

void test_destroy_clears_dice_workflow() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();

    ui.request_destroy();

    check(
        ui.destroy_session() ==
            cryptomachine::UIActionStatus::Success,
        "destroy_session must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "destroyed workflow must return Home"
    );

    check(
        ui.word_count() == 0,
        "destroyed workflow must clear word count"
    );

    check(
        ui.expected_word_input() == 0,
        "destroyed workflow must clear expected input"
    );

    check(
        !ui.dice_session_active(),
        "destroyed workflow must clear dice session"
    );

    check(
        !ui.mnemonic_generated(),
        "destroyed workflow must clear mnemonic state"
    );

    check(
        !ui.sensitive_data_present(),
        "destroyed workflow must clear sensitive state"
    );
}

void test_destroy_after_mnemonic_clears_state() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(12);
    ui.begin_dice_entry();
    ui.dice_entry_to_review();
    ui.shake_confirm_complete();
    ui.dice_complete_continue(false);
    ui.generate_mnemonic();

    check(
        ui.mnemonic_generated(),
        "mnemonic workflow must mark mnemonic generated"
    );

    ui.mnemonic_words_complete();
    ui.mnemonic_review_finish();

    check(
        ui.state() ==
            cryptomachine::UIState::SessionDestroyConfirm,
        "mnemonic finish must request destruction"
    );

    ui.destroy_session();

    check(
        ui.state() == cryptomachine::UIState::Home,
        "mnemonic destruction must return Home"
    );

    check(
        !ui.mnemonic_generated(),
        "mnemonic destruction must clear generated state"
    );

    check(
        !ui.dice_session_active(),
        "mnemonic destruction must clear dice session"
    );

    check(
        ui.word_count() == 0,
        "mnemonic destruction must clear word count"
    );

    check(
        !ui.sensitive_data_present(),
        "mnemonic destruction must clear sensitive state"
    );
}

void test_old_workflow_cannot_continue_after_destroy() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(12);
    ui.begin_dice_entry();

    ui.request_destroy();
    ui.destroy_session();

    check(
        ui.dice_entry_to_review() ==
            cryptomachine::UIActionStatus::InvalidState,
        "destroyed dice workflow must not continue"
    );

    check(
        ui.generate_mnemonic() ==
            cryptomachine::UIActionStatus::InvalidState,
        "destroyed workflow must not generate mnemonic"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "destroyed workflow must remain at Home"
    );
}

void test_cross_workflow_jump_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();

    check(
        ui.open_final_word_tool() ==
            cryptomachine::UIActionStatus::InvalidState,
        "cross-jump to final-word tool must be rejected"
    );

    check(
        ui.open_validate_phrase() ==
            cryptomachine::UIActionStatus::InvalidState,
        "cross-jump to validation must be rejected"
    );

    check(
        ui.open_about_security() ==
            cryptomachine::UIActionStatus::InvalidState,
        "cross-jump to About must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::DiceEntry,
        "cross-workflow rejection must preserve DiceEntry"
    );
}

void test_final_word_invalid_mode_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_final_word_tool();

    check(
        ui.choose_final_word_mode(18) ==
            cryptomachine::UIActionStatus::InvalidValue,
        "invalid final-word mode must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::FinalWordMode,
        "invalid final-word mode must preserve state"
    );

    check(
        ui.expected_word_input() == 0,
        "invalid final-word mode must not set expected input"
    );
}

void test_final_word_destroy_clears_input_state() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_final_word_tool();
    ui.choose_final_word_mode(24);

    check(
        ui.expected_word_input() == 23,
        "24-word final-word mode must expect 23 inputs"
    );

    check(
        ui.sensitive_data_present(),
        "final-word entry must be sensitive"
    );

    ui.request_destroy();
    ui.destroy_session();

    check(
        ui.state() == cryptomachine::UIState::Home,
        "final-word destruction must return Home"
    );

    check(
        ui.expected_word_input() == 0,
        "final-word destruction must clear expected input"
    );

    check(
        !ui.sensitive_data_present(),
        "final-word destruction must clear sensitive state"
    );
}

void test_validation_cannot_complete_without_word_count() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_validate_phrase();

    check(
        ui.validation_complete() ==
            cryptomachine::UIActionStatus::
                ValidationWordCountNotSelected,
        "validation without word count must be rejected"
    );

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseEntry,
        "rejected validation must preserve entry state"
    );
}

void test_invalid_validation_count_rejected() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_validate_phrase();

    check(
        ui.choose_validation_word_count(15) ==
            cryptomachine::UIActionStatus::InvalidValue,
        "invalid validation word count must be rejected"
    );

    check(
        ui.expected_word_input() == 0,
        "invalid validation count must not set input count"
    );
}

void test_validation_exit_clears_sensitive_state() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_validate_phrase();
    ui.choose_validation_word_count(24);
    ui.validation_complete();

    check(
        ui.state() ==
            cryptomachine::UIState::ValidatePhraseResult,
        "validation must reach result state"
    );

    check(
        ui.sensitive_data_present(),
        "validation result must remain sensitive"
    );

    check(
        ui.leave_sensitive_workflow_to_home() ==
            cryptomachine::UIActionStatus::Success,
        "sensitive validation exit must succeed"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "validation exit must return Home"
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

void test_sanity_restart_clears_old_sensitive_state() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();
    ui.open_create_from_dice();
    ui.choose_dice_word_count(24);
    ui.begin_dice_entry();
    ui.dice_entry_to_review();
    ui.shake_confirm_complete();
    ui.dice_complete_continue(true);

    check(
        ui.sensitive_data_present(),
        "sanity warning workflow must contain sensitive state"
    );

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
        "sanity restart must preserve active mode"
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

void test_boot_cannot_run_twice() {
    cryptomachine::UIStateMachine ui;

    ui.boot_complete();

    check(
        ui.boot_complete() ==
            cryptomachine::UIActionStatus::InvalidState,
        "boot_complete must not run twice"
    );

    check(
        ui.state() == cryptomachine::UIState::Home,
        "rejected second boot must preserve Home"
    );
}

}  // namespace

int main() {
    test_generate_from_home_rejected();
    test_begin_dice_without_session_rejected();
    test_invalid_dice_word_count_rejected();
    test_generate_too_early_rejected();
    test_destroy_from_home_rejected();
    test_double_destroy_request_rejected();
    test_destroy_go_back_preserves_session();
    test_destroy_clears_dice_workflow();
    test_destroy_after_mnemonic_clears_state();
    test_old_workflow_cannot_continue_after_destroy();
    test_cross_workflow_jump_rejected();
    test_final_word_invalid_mode_rejected();
    test_final_word_destroy_clears_input_state();
    test_validation_cannot_complete_without_word_count();
    test_invalid_validation_count_rejected();
    test_validation_exit_clears_sensitive_state();
    test_sanity_restart_clears_old_sensitive_state();
    test_boot_cannot_run_twice();

    if (failures != 0) {
        std::cerr
            << failures
            << " UI abuse/security test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All 18 C++ UI abuse/security-navigation tests passed.\n";

    return 0;
}