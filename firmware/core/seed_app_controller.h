#pragma once

#include "dice_sanity.h"
#include "seed_ceremony.h"
#include "ui_state_machine.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace cryptomachine {

enum class SeedAppStatus {
    Success = 0,

    InvalidState,
    InvalidValue,
    UnsupportedWordCount,

    NoActiveDiceSession,

    InvalidShakeSize,
    InvalidDiceValue,
    NothingToUndo,
    SessionComplete,

    NoPendingShake,
    PendingShakeExists,

    SanityAnalysisFailed,
    NoSanityReport,

    AlreadyGenerated,
    IncompleteDice,
    SeedGenerationFailed,

    InternalStateError,
};

class SeedAppController {
public:
    SeedAppController() = default;

    ~SeedAppController();

    SeedAppController(
        const SeedAppController&
    ) = delete;

    SeedAppController& operator=(
        const SeedAppController&
    ) = delete;

    SeedAppController(
        SeedAppController&&
    ) = delete;

    SeedAppController& operator=(
        SeedAppController&&
    ) = delete;

    // --------------------------------------------------------
    // READ-ONLY APPLICATION STATE
    // --------------------------------------------------------

    UIState state() const;

    std::size_t word_count() const;

    std::size_t shake_count() const;

    std::size_t total_shakes() const;

    std::size_t remaining_shakes() const;

    bool ceremony_complete() const;

    bool generated() const;

    bool sensitive_data_present() const;

    // --------------------------------------------------------
    // BOOT / START DICE CEREMONY
    // --------------------------------------------------------

    SeedAppStatus boot_complete();

    SeedAppStatus open_create_from_dice();

    SeedAppStatus choose_dice_word_count(
        std::size_t word_count
    );

    SeedAppStatus dice_word_count_back();

    SeedAppStatus begin_dice_entry();

    // --------------------------------------------------------
    // FIVE-DIE ENTRY / REVIEW
    // --------------------------------------------------------

    // Copies exactly five validated dice faces into the
    // controller's fixed pending-shake buffer and advances
    // the UI to the shake-review state.
    SeedAppStatus enter_shake_for_review(
        std::string_view results
    );

    bool has_pending_shake() const;

    // Non-owning view of the five pending dice faces.
    // Empty when no shake is awaiting confirmation.
    std::string_view pending_shake() const;

    // Reject the pending shake, securely wipe it, and
    // return to dice entry.
    SeedAppStatus reenter_pending_shake();

    // Commit the pending shake to SeedCeremony.
    //
    // If more shakes are required, returns to DiceEntry.
    // If the ceremony becomes complete, the controller
    // performs the pre-generation dice sanity analysis and
    // advances to DiceComplete.
    SeedAppStatus confirm_pending_shake();

    SeedAppStatus undo_last_shake();

    // --------------------------------------------------------
    // COMPLETION / SANITY
    // --------------------------------------------------------

    bool sanity_report_ready() const;

    // Non-owning pointer owned by this controller.
    // nullptr until the dice ceremony is complete and
    // sanity analysis has succeeded.
    const FiveDiceSanity* sanity_report() const;

    // Advances from DiceComplete either to the sanity
    // warning screen or directly to generation confirmation,
    // based on the real stored sanity report.
    SeedAppStatus continue_from_dice_complete();

    SeedAppStatus sanity_continue_anyway();

    // Securely wipes the old dice history and sanity report,
    // preserving the selected 12/24-word ceremony mode.
    SeedAppStatus sanity_restart();

    SeedAppStatus generate_back();

    // --------------------------------------------------------
    // GENERATION / REVIEW
    // --------------------------------------------------------

    // Calls the real SeedCeremony generator first.
    // The UI enters MnemonicWordView only after successful
    // seed generation.
    SeedAppStatus generate_mnemonic();

    // Non-owning pointer to the ceremony-owned result.
    // nullptr until generation succeeds.
    const SeedResult* seed_result() const;

    SeedAppStatus mnemonic_words_complete();

    SeedAppStatus mnemonic_review_finish();

    // --------------------------------------------------------
    // SECURE DESTRUCTION
    // --------------------------------------------------------

    SeedAppStatus request_destroy();

    SeedAppStatus destroy_go_back();

    // Securely wipes:
    //   - pending shake
    //   - pre-generation sanity report
    //   - generated mnemonic/result
    //   - complete dice history
    //
    // Then returns the UI to Home.
    SeedAppStatus destroy_session();

    // Emergency path for critical runtime faults.
    // This is intentionally unconditional and does not
    // require SessionDestroyConfirm or any other UI state.
    //
    // Securely wipes all controller-owned sensitive session
    // material and resets application state to Home.
    void emergency_destroy_session();

private:
    // Single controller-owned destruction primitive used by both
    // the normal confirmed-destroy path and unconditional emergency
    // destruction. Keep all sensitive controller state here so the
    // two public destruction routes cannot drift apart.
    void destroy_sensitive_state();

    void wipe_pending_shake();

    void wipe_sanity_report();

    UIStateMachine ui_;
    SeedCeremony ceremony_;

    std::array<char, kDiceCount> pending_shake_{};
    bool pending_shake_valid_ = false;

    FiveDiceSanity sanity_{};
    bool sanity_ready_ = false;
};

}  // namespace cryptomachine
