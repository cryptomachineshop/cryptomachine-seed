#pragma once

#include <cstddef>

namespace cryptomachine {

enum class UIState {
    Boot = 0,
    Home,

    DiceWordCount,
    DiceIntro,
    DiceEntry,
    DiceShakeReview,
    DiceComplete,
    DiceSanityWarning,
    DiceGenerateConfirm,
    MnemonicWordView,
    MnemonicFullReview,
    EntropyDetails,
    SessionDestroyConfirm,

    FinalWordMode,
    FinalWordEntry,
    FinalWordResults,

    ValidatePhraseEntry,
    ValidatePhraseResult,

    AboutSecurity,
};

enum class UIActionStatus {
    Success = 0,

    InvalidState,
    InvalidValue,

    NoActiveDiceSession,
    MnemonicNotGenerated,

    AlreadyInDestroyConfirmation,
    NoDestroyReturnState,

    ValidationWordCountNotSelected,

    NotSensitiveWorkflow,
};

bool is_sensitive_ui_state(
    UIState state
);

class UIStateMachine {
public:
    UIStateMachine() = default;

    UIState state() const;

    bool is_sensitive_state() const;

    std::size_t word_count() const;

    std::size_t expected_word_input() const;

    bool dice_session_active() const;

    bool mnemonic_generated() const;

    bool sensitive_data_present() const;

    // BOOT / HOME

    UIActionStatus boot_complete();

    UIActionStatus open_create_from_dice();

    UIActionStatus open_final_word_tool();

    UIActionStatus open_validate_phrase();

    UIActionStatus open_about_security();

    UIActionStatus about_back();

    // DICE WORKFLOW

    UIActionStatus choose_dice_word_count(
        std::size_t word_count
    );

    UIActionStatus dice_word_count_back();

    UIActionStatus begin_dice_entry();

    UIActionStatus dice_entry_to_review();

    UIActionStatus shake_review_reenter();

    UIActionStatus shake_confirm_more_required();

    UIActionStatus shake_confirm_complete();

    UIActionStatus dice_complete_continue(
        bool warnings
    );

    UIActionStatus sanity_continue_anyway();

    UIActionStatus sanity_restart();

    UIActionStatus generate_back();

    UIActionStatus generate_mnemonic();

    UIActionStatus mnemonic_words_complete();

    UIActionStatus mnemonic_review_finish();

    UIActionStatus entropy_details_open();

    UIActionStatus entropy_details_back();

    // FINAL WORD TOOL

    UIActionStatus choose_final_word_mode(
        std::size_t total_words
    );

    UIActionStatus final_word_mode_back();

    UIActionStatus final_word_entry_complete();

    UIActionStatus final_word_results_back();

    UIActionStatus final_word_results_home();

    // VALIDATION TOOL

    UIActionStatus choose_validation_word_count(
        std::size_t word_count
    );

    UIActionStatus validation_complete();

    UIActionStatus validation_review_words();

    UIActionStatus validation_start_over();

    UIActionStatus validation_result_home();

    // DESTRUCTION / LEAVING SENSITIVE WORKFLOWS

    UIActionStatus request_destroy();

    UIActionStatus destroy_go_back();

    UIActionStatus destroy_session();

    UIActionStatus leave_sensitive_workflow_to_home();

    // Unconditional reset used only after a critical runtime
    // fault has already required the surrounding application
    // to destroy all sensitive material.
    void emergency_reset_to_home();

private:
    bool require_state(
        UIState allowed
    ) const;

    void set_state(
        UIState state
    );

    void mark_sensitive();

    void clear_sensitive_session();

    UIState state_ = UIState::Boot;

    // Zero represents "not selected".
    std::size_t word_count_ = 0;
    std::size_t expected_word_input_ = 0;

    bool dice_session_active_ = false;
    bool mnemonic_generated_ = false;
    bool sensitive_data_present_ = false;

    UIState destroy_return_state_ = UIState::Boot;
    bool has_destroy_return_state_ = false;
};

}  // namespace cryptomachine
