#include "ui_state_machine.h"

namespace cryptomachine {

bool is_sensitive_ui_state(
    UIState state
) {
    switch (state) {
        case UIState::DiceEntry:
        case UIState::DiceShakeReview:
        case UIState::DiceComplete:
        case UIState::DiceSanityWarning:
        case UIState::DiceGenerateConfirm:
        case UIState::MnemonicWordView:
        case UIState::MnemonicFullReview:
        case UIState::SessionDestroyConfirm:
        case UIState::FinalWordEntry:
        case UIState::FinalWordResults:
        case UIState::ValidatePhraseEntry:
        case UIState::ValidatePhraseResult:
            return true;

        default:
            return false;
    }
}

UIState UIStateMachine::state() const {
    return state_;
}

bool UIStateMachine::is_sensitive_state() const {
    return is_sensitive_ui_state(state_);
}

std::size_t UIStateMachine::word_count() const {
    return word_count_;
}

std::size_t UIStateMachine::expected_word_input() const {
    return expected_word_input_;
}

bool UIStateMachine::dice_session_active() const {
    return dice_session_active_;
}

bool UIStateMachine::mnemonic_generated() const {
    return mnemonic_generated_;
}

bool UIStateMachine::sensitive_data_present() const {
    return sensitive_data_present_;
}

bool UIStateMachine::require_state(
    UIState allowed
) const {
    return state_ == allowed;
}

void UIStateMachine::set_state(
    UIState state
) {
    state_ = state;
}

void UIStateMachine::mark_sensitive() {
    sensitive_data_present_ = true;
}

void UIStateMachine::clear_sensitive_session() {
    word_count_ = 0;
    expected_word_input_ = 0;

    dice_session_active_ = false;
    mnemonic_generated_ = false;
    sensitive_data_present_ = false;

    destroy_return_state_ = UIState::Boot;
    has_destroy_return_state_ = false;
}

// ------------------------------------------------------------
// BOOT / HOME
// ------------------------------------------------------------

UIActionStatus UIStateMachine::boot_complete() {
    if (!require_state(UIState::Boot)) {
        return UIActionStatus::InvalidState;
    }

    clear_sensitive_session();
    set_state(UIState::Home);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::open_create_from_dice() {
    if (!require_state(UIState::Home)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceWordCount);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::open_final_word_tool() {
    if (!require_state(UIState::Home)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::FinalWordMode);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::open_validate_phrase() {
    if (!require_state(UIState::Home)) {
        return UIActionStatus::InvalidState;
    }

    expected_word_input_ = 0;
    mark_sensitive();

    set_state(UIState::ValidatePhraseEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::open_about_security() {
    if (!require_state(UIState::Home)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::AboutSecurity);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::about_back() {
    if (!require_state(UIState::AboutSecurity)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::Home);

    return UIActionStatus::Success;
}

// ------------------------------------------------------------
// DICE WORKFLOW
// ------------------------------------------------------------

UIActionStatus UIStateMachine::choose_dice_word_count(
    std::size_t word_count
) {
    if (!require_state(UIState::DiceWordCount)) {
        return UIActionStatus::InvalidState;
    }

    if (word_count != 12 && word_count != 24) {
        return UIActionStatus::InvalidValue;
    }

    word_count_ = word_count;
    dice_session_active_ = true;
    mnemonic_generated_ = false;
    sensitive_data_present_ = false;

    set_state(UIState::DiceIntro);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::dice_word_count_back() {
    if (!require_state(UIState::DiceWordCount)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::Home);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::begin_dice_entry() {
    if (!require_state(UIState::DiceIntro)) {
        return UIActionStatus::InvalidState;
    }

    if (!dice_session_active_) {
        return UIActionStatus::NoActiveDiceSession;
    }

    mark_sensitive();
    set_state(UIState::DiceEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::dice_entry_to_review() {
    if (!require_state(UIState::DiceEntry)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceShakeReview);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::shake_review_reenter() {
    if (!require_state(UIState::DiceShakeReview)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::shake_confirm_more_required() {
    if (!require_state(UIState::DiceShakeReview)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::shake_confirm_complete() {
    if (!require_state(UIState::DiceShakeReview)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceComplete);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::dice_complete_continue(
    bool warnings
) {
    if (!require_state(UIState::DiceComplete)) {
        return UIActionStatus::InvalidState;
    }

    if (warnings) {
        set_state(UIState::DiceSanityWarning);
    } else {
        set_state(UIState::DiceGenerateConfirm);
    }

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::sanity_continue_anyway() {
    if (!require_state(UIState::DiceSanityWarning)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceGenerateConfirm);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::sanity_restart() {
    if (!require_state(UIState::DiceSanityWarning)) {
        return UIActionStatus::InvalidState;
    }

    const std::size_t previous_word_count =
        word_count_;

    clear_sensitive_session();

    word_count_ = previous_word_count;
    dice_session_active_ = true;

    set_state(UIState::DiceIntro);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::generate_back() {
    if (!require_state(UIState::DiceGenerateConfirm)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::DiceComplete);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::generate_mnemonic() {
    if (!require_state(UIState::DiceGenerateConfirm)) {
        return UIActionStatus::InvalidState;
    }

    if (!dice_session_active_) {
        return UIActionStatus::NoActiveDiceSession;
    }

    mnemonic_generated_ = true;
    mark_sensitive();

    set_state(UIState::MnemonicWordView);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::mnemonic_words_complete() {
    if (!require_state(UIState::MnemonicWordView)) {
        return UIActionStatus::InvalidState;
    }

    if (!mnemonic_generated_) {
        return UIActionStatus::MnemonicNotGenerated;
    }

    set_state(UIState::MnemonicFullReview);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::mnemonic_review_finish() {
    if (!require_state(UIState::MnemonicFullReview)) {
        return UIActionStatus::InvalidState;
    }

    return request_destroy();
}

// ------------------------------------------------------------
// FINAL WORD TOOL
// ------------------------------------------------------------

UIActionStatus UIStateMachine::choose_final_word_mode(
    std::size_t total_words
) {
    if (!require_state(UIState::FinalWordMode)) {
        return UIActionStatus::InvalidState;
    }

    if (total_words == 12) {
        expected_word_input_ = 11;
    } else if (total_words == 24) {
        expected_word_input_ = 23;
    } else {
        return UIActionStatus::InvalidValue;
    }

    mark_sensitive();
    set_state(UIState::FinalWordEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::final_word_mode_back() {
    if (!require_state(UIState::FinalWordMode)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::Home);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::final_word_entry_complete() {
    if (!require_state(UIState::FinalWordEntry)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::FinalWordResults);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::final_word_results_back() {
    if (!require_state(UIState::FinalWordResults)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::FinalWordEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::final_word_results_home() {
    if (!require_state(UIState::FinalWordResults)) {
        return UIActionStatus::InvalidState;
    }

    clear_sensitive_session();
    set_state(UIState::Home);

    return UIActionStatus::Success;
}

// ------------------------------------------------------------
// VALIDATION TOOL
// ------------------------------------------------------------

UIActionStatus UIStateMachine::choose_validation_word_count(
    std::size_t word_count
) {
    if (!require_state(UIState::ValidatePhraseEntry)) {
        return UIActionStatus::InvalidState;
    }

    if (word_count != 12 && word_count != 24) {
        return UIActionStatus::InvalidValue;
    }

    expected_word_input_ = word_count;
    mark_sensitive();

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::validation_complete() {
    if (!require_state(UIState::ValidatePhraseEntry)) {
        return UIActionStatus::InvalidState;
    }

    if (
        expected_word_input_ != 12 &&
        expected_word_input_ != 24
    ) {
        return UIActionStatus::
            ValidationWordCountNotSelected;
    }

    set_state(UIState::ValidatePhraseResult);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::validation_review_words() {
    if (!require_state(UIState::ValidatePhraseResult)) {
        return UIActionStatus::InvalidState;
    }

    set_state(UIState::ValidatePhraseEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::validation_start_over() {
    if (!require_state(UIState::ValidatePhraseResult)) {
        return UIActionStatus::InvalidState;
    }

    expected_word_input_ = 0;
    sensitive_data_present_ = false;

    set_state(UIState::ValidatePhraseEntry);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::validation_result_home() {
    if (!require_state(UIState::ValidatePhraseResult)) {
        return UIActionStatus::InvalidState;
    }

    clear_sensitive_session();
    set_state(UIState::Home);

    return UIActionStatus::Success;
}

// ------------------------------------------------------------
// DESTRUCTION / LEAVING SENSITIVE WORKFLOWS
// ------------------------------------------------------------

UIActionStatus UIStateMachine::request_destroy() {
    if (!is_sensitive_ui_state(state_)) {
        return UIActionStatus::NotSensitiveWorkflow;
    }

    if (state_ == UIState::SessionDestroyConfirm) {
        return UIActionStatus::
            AlreadyInDestroyConfirmation;
    }

    destroy_return_state_ = state_;
    has_destroy_return_state_ = true;

    set_state(UIState::SessionDestroyConfirm);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::destroy_go_back() {
    if (!require_state(UIState::SessionDestroyConfirm)) {
        return UIActionStatus::InvalidState;
    }

    if (!has_destroy_return_state_) {
        return UIActionStatus::NoDestroyReturnState;
    }

    const UIState previous_state =
        destroy_return_state_;

    destroy_return_state_ = UIState::Boot;
    has_destroy_return_state_ = false;

    set_state(previous_state);

    return UIActionStatus::Success;
}

UIActionStatus UIStateMachine::destroy_session() {
    if (!require_state(UIState::SessionDestroyConfirm)) {
        return UIActionStatus::InvalidState;
    }

    clear_sensitive_session();
    set_state(UIState::Home);

    return UIActionStatus::Success;
}

UIActionStatus
UIStateMachine::leave_sensitive_workflow_to_home() {
    if (!is_sensitive_ui_state(state_)) {
        return UIActionStatus::NotSensitiveWorkflow;
    }

    clear_sensitive_session();
    set_state(UIState::Home);

    return UIActionStatus::Success;
}

void UIStateMachine::emergency_reset_to_home() {
    clear_sensitive_session();
    set_state(UIState::Home);
}

}  // namespace cryptomachine
