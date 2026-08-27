#include "seed_app_controller.h"

#include "secure_zero.h"

namespace cryptomachine {
namespace {

SeedAppStatus map_ui_status(
    UIActionStatus status
) {
    switch (status) {
        case UIActionStatus::Success:
            return SeedAppStatus::Success;

        case UIActionStatus::InvalidState:
        case UIActionStatus::AlreadyInDestroyConfirmation:
        case UIActionStatus::NoDestroyReturnState:
        case UIActionStatus::MnemonicNotGenerated:
        case UIActionStatus::ValidationWordCountNotSelected:
        case UIActionStatus::NotSensitiveWorkflow:
            return SeedAppStatus::InvalidState;

        case UIActionStatus::InvalidValue:
            return SeedAppStatus::InvalidValue;

        case UIActionStatus::NoActiveDiceSession:
            return SeedAppStatus::NoActiveDiceSession;
    }

    return SeedAppStatus::InternalStateError;
}

SeedAppStatus map_ceremony_status(
    SeedCeremonyStatus status
) {
    switch (status) {
        case SeedCeremonyStatus::Success:
            return SeedAppStatus::Success;

        case SeedCeremonyStatus::UnsupportedWordCount:
            return SeedAppStatus::UnsupportedWordCount;

        case SeedCeremonyStatus::NotInitialized:
            return SeedAppStatus::NoActiveDiceSession;

        case SeedCeremonyStatus::AlreadyGenerated:
            return SeedAppStatus::AlreadyGenerated;

        case SeedCeremonyStatus::IncompleteDice:
            return SeedAppStatus::IncompleteDice;

        case SeedCeremonyStatus::SessionComplete:
            return SeedAppStatus::SessionComplete;

        case SeedCeremonyStatus::InvalidShakeSize:
            return SeedAppStatus::InvalidShakeSize;

        case SeedCeremonyStatus::InvalidDiceValue:
            return SeedAppStatus::InvalidDiceValue;

        case SeedCeremonyStatus::NothingToUndo:
            return SeedAppStatus::NothingToUndo;

        case SeedCeremonyStatus::SeedGenerationFailed:
            return SeedAppStatus::SeedGenerationFailed;
    }

    return SeedAppStatus::InternalStateError;
}

}  // namespace

SeedAppController::~SeedAppController() {
    wipe_pending_shake();
    wipe_sanity_report();

    ceremony_.destroy_session();
}

UIState SeedAppController::state() const {
    return ui_.state();
}

std::size_t SeedAppController::word_count() const {
    return ui_.word_count();
}

std::size_t SeedAppController::shake_count() const {
    return ceremony_.shake_count();
}

std::size_t SeedAppController::total_shakes() const {
    return ceremony_.total_shakes();
}

std::size_t SeedAppController::remaining_shakes() const {
    return ceremony_.remaining_shakes();
}

bool SeedAppController::ceremony_complete() const {
    return ceremony_.complete();
}

bool SeedAppController::generated() const {
    return ceremony_.generated();
}

bool SeedAppController::sensitive_data_present() const {
    return ui_.sensitive_data_present();
}

// ------------------------------------------------------------
// BOOT / START DICE CEREMONY
// ------------------------------------------------------------

SeedAppStatus SeedAppController::boot_complete() {
    return map_ui_status(
        ui_.boot_complete()
    );
}

SeedAppStatus SeedAppController::open_create_from_dice() {
    return map_ui_status(
        ui_.open_create_from_dice()
    );
}

SeedAppStatus SeedAppController::choose_dice_word_count(
    std::size_t word_count
) {
    if (ui_.state() != UIState::DiceWordCount) {
        return SeedAppStatus::InvalidState;
    }

    if (
        word_count != kWordCount12 &&
        word_count != kWordCount24
    ) {
        return SeedAppStatus::InvalidValue;
    }

    wipe_pending_shake();
    wipe_sanity_report();

    const SeedCeremonyStatus ceremony_status =
        ceremony_.initialize(word_count);

    if (ceremony_status != SeedCeremonyStatus::Success) {
        return map_ceremony_status(
            ceremony_status
        );
    }

    const UIActionStatus ui_status =
        ui_.choose_dice_word_count(
            word_count
        );

    if (ui_status != UIActionStatus::Success) {
        ceremony_.destroy_session();

        return map_ui_status(
            ui_status
        );
    }

    return SeedAppStatus::Success;
}

SeedAppStatus SeedAppController::dice_word_count_back() {
    return map_ui_status(
        ui_.dice_word_count_back()
    );
}

SeedAppStatus SeedAppController::begin_dice_entry() {
    if (ui_.state() != UIState::DiceIntro) {
        return SeedAppStatus::InvalidState;
    }

    if (
        ceremony_.word_count() != kWordCount12 &&
        ceremony_.word_count() != kWordCount24
    ) {
        return SeedAppStatus::NoActiveDiceSession;
    }

    return map_ui_status(
        ui_.begin_dice_entry()
    );
}

// ------------------------------------------------------------
// FIVE-DIE ENTRY / REVIEW
// ------------------------------------------------------------

SeedAppStatus SeedAppController::enter_shake_for_review(
    std::string_view results
) {
    if (ui_.state() != UIState::DiceEntry) {
        return SeedAppStatus::InvalidState;
    }

    if (pending_shake_valid_) {
        return SeedAppStatus::PendingShakeExists;
    }

    if (results.size() != kDiceCount) {
        return SeedAppStatus::InvalidShakeSize;
    }

    for (char value : results) {
        if (!valid_dice_char(value)) {
            return SeedAppStatus::InvalidDiceValue;
        }
    }

    for (std::size_t i = 0; i < kDiceCount; ++i) {
        pending_shake_[i] = results[i];
    }

    pending_shake_valid_ = true;

    const UIActionStatus ui_status =
        ui_.dice_entry_to_review();

    if (ui_status != UIActionStatus::Success) {
        wipe_pending_shake();

        return map_ui_status(
            ui_status
        );
    }

    return SeedAppStatus::Success;
}

bool SeedAppController::has_pending_shake() const {
    return pending_shake_valid_;
}

std::string_view SeedAppController::pending_shake() const {
    if (!pending_shake_valid_) {
        return {};
    }

    return std::string_view(
        pending_shake_.data(),
        pending_shake_.size()
    );
}

SeedAppStatus SeedAppController::reenter_pending_shake() {
    if (ui_.state() != UIState::DiceShakeReview) {
        return SeedAppStatus::InvalidState;
    }

    if (!pending_shake_valid_) {
        return SeedAppStatus::NoPendingShake;
    }

    wipe_pending_shake();

    return map_ui_status(
        ui_.shake_review_reenter()
    );
}

SeedAppStatus SeedAppController::confirm_pending_shake() {
    if (ui_.state() != UIState::DiceShakeReview) {
        return SeedAppStatus::InvalidState;
    }

    if (!pending_shake_valid_) {
        return SeedAppStatus::NoPendingShake;
    }

    const std::string_view shake(
        pending_shake_.data(),
        pending_shake_.size()
    );

    const SeedCeremonyStatus ceremony_status =
        ceremony_.add_shake(shake);

    if (ceremony_status != SeedCeremonyStatus::Success) {
        return map_ceremony_status(
            ceremony_status
        );
    }

    wipe_pending_shake();

    if (!ceremony_.complete()) {
        return map_ui_status(
            ui_.shake_confirm_more_required()
        );
    }

    wipe_sanity_report();

    if (!analyze_five_dice(
            ceremony_.canonical_dice(),
            sanity_
        )) {
        wipe_sanity_report();

        return SeedAppStatus::SanityAnalysisFailed;
    }

    sanity_ready_ = true;

    return map_ui_status(
        ui_.shake_confirm_complete()
    );
}

SeedAppStatus SeedAppController::undo_last_shake() {
    if (ui_.state() != UIState::DiceEntry) {
        return SeedAppStatus::InvalidState;
    }

    if (pending_shake_valid_) {
        return SeedAppStatus::PendingShakeExists;
    }

    return map_ceremony_status(
        ceremony_.undo_last_shake()
    );
}

// ------------------------------------------------------------
// COMPLETION / SANITY
// ------------------------------------------------------------

bool SeedAppController::sanity_report_ready() const {
    return sanity_ready_;
}

const FiveDiceSanity*
SeedAppController::sanity_report() const {
    if (!sanity_ready_) {
        return nullptr;
    }

    return &sanity_;
}

SeedAppStatus
SeedAppController::continue_from_dice_complete() {
    if (ui_.state() != UIState::DiceComplete) {
        return SeedAppStatus::InvalidState;
    }

    if (!sanity_ready_) {
        return SeedAppStatus::NoSanityReport;
    }

    const bool warnings =
        sanity_.warning_count != 0;

    return map_ui_status(
        ui_.dice_complete_continue(
            warnings
        )
    );
}

SeedAppStatus
SeedAppController::sanity_continue_anyway() {
    return map_ui_status(
        ui_.sanity_continue_anyway()
    );
}

SeedAppStatus SeedAppController::sanity_restart() {
    if (ui_.state() != UIState::DiceSanityWarning) {
        return SeedAppStatus::InvalidState;
    }

    wipe_pending_shake();
    wipe_sanity_report();

    ceremony_.destroy_session();

    return map_ui_status(
        ui_.sanity_restart()
    );
}

SeedAppStatus SeedAppController::generate_back() {
    return map_ui_status(
        ui_.generate_back()
    );
}

// ------------------------------------------------------------
// GENERATION / REVIEW
// ------------------------------------------------------------

SeedAppStatus SeedAppController::generate_mnemonic() {
    if (ui_.state() != UIState::DiceGenerateConfirm) {
        return SeedAppStatus::InvalidState;
    }

    const SeedCeremonyStatus ceremony_status =
        ceremony_.generate();

    if (ceremony_status != SeedCeremonyStatus::Success) {
        return map_ceremony_status(
            ceremony_status
        );
    }

    const UIActionStatus ui_status =
        ui_.generate_mnemonic();

    if (ui_status != UIActionStatus::Success) {
        ceremony_.destroy_session();

        return map_ui_status(
            ui_status
        );
    }

    return SeedAppStatus::Success;
}

const SeedResult*
SeedAppController::seed_result() const {
    return ceremony_.result();
}

SeedAppStatus
SeedAppController::mnemonic_words_complete() {
    return map_ui_status(
        ui_.mnemonic_words_complete()
    );
}

SeedAppStatus
SeedAppController::mnemonic_review_finish() {
    return map_ui_status(
        ui_.mnemonic_review_finish()
    );
}

// ------------------------------------------------------------
// SECURE DESTRUCTION
// ------------------------------------------------------------

SeedAppStatus SeedAppController::request_destroy() {
    return map_ui_status(
        ui_.request_destroy()
    );
}

SeedAppStatus SeedAppController::destroy_go_back() {
    return map_ui_status(
        ui_.destroy_go_back()
    );
}

SeedAppStatus SeedAppController::destroy_session() {
    if (
        ui_.state() !=
        UIState::SessionDestroyConfirm
    ) {
        return SeedAppStatus::InvalidState;
    }

    wipe_pending_shake();
    wipe_sanity_report();

    ceremony_.destroy_session();

    return map_ui_status(
        ui_.destroy_session()
    );
}

void SeedAppController::emergency_destroy_session() {
    // Do not consult UI state here. A critical hardware/runtime
    // fault may mean the normal UI flow can no longer be trusted.
    wipe_pending_shake();
    wipe_sanity_report();

    ceremony_.destroy_session();
    ui_.emergency_reset_to_home();
}

void SeedAppController::wipe_pending_shake() {
    secure_zero(
        pending_shake_.data(),
        pending_shake_.size()
    );

    pending_shake_valid_ = false;
}

void SeedAppController::wipe_sanity_report() {
    destroy_five_dice_sanity(
        sanity_
    );

    sanity_ready_ = false;
}

}  // namespace cryptomachine