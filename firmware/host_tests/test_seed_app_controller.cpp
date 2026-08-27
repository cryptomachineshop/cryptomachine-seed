#include "bip39.h"
#include "bip39_wordlist.h"
#include "seed_app_controller.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

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

std::string mnemonic_string(
    const cryptomachine::SeedResult& result
) {
    return cryptomachine::bip39_mnemonic_to_string(
        result.mnemonic,
        cryptomachine::kBip39EnglishWordlist
    );
}

void test_real_12_word_workflow() {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    const std::string expected =
        "hole luggage safe present express tragic orbit shed "
        "switch metal identify path";

    cryptomachine::SeedAppController app;

    check(
        app.state() == cryptomachine::UIState::Boot,
        "controller must start in Boot"
    );

    check(
        app.boot_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "boot must succeed"
    );

    check(
        app.open_create_from_dice() ==
            cryptomachine::SeedAppStatus::Success,
        "create-from-dice must open"
    );

    check(
        app.choose_dice_word_count(12) ==
            cryptomachine::SeedAppStatus::Success,
        "12-word mode must initialize real ceremony"
    );

    check(
        app.total_shakes() == 10,
        "12-word mode must require 10 shakes"
    );

    check(
        app.begin_dice_entry() ==
            cryptomachine::SeedAppStatus::Success,
        "dice entry must begin"
    );

    // Exercise the temporary review buffer before recording
    // any real ceremony data.
    check(
        app.enter_shake_for_review("12345") ==
            cryptomachine::SeedAppStatus::Success,
        "temporary shake must enter review"
    );

    check(
        app.has_pending_shake(),
        "reviewed shake must exist in pending buffer"
    );

    check(
        app.pending_shake() == "12345",
        "pending shake must preserve fixed D1-D5 order"
    );

    check(
        app.shake_count() == 0,
        "pending shake must not yet be committed"
    );

    check(
        app.reenter_pending_shake() ==
            cryptomachine::SeedAppStatus::Success,
        "re-enter action must reject pending shake"
    );

    check(
        !app.has_pending_shake(),
        "rejected pending shake must be cleared"
    );

    check(
        app.pending_shake().empty(),
        "rejected pending shake view must be empty"
    );

    check(
        app.shake_count() == 0,
        "rejected shake must never reach ceremony history"
    );

    // Enter the real known-vector ceremony.
    for (
        std::size_t offset = 0;
        offset < dice.size();
        offset += cryptomachine::kDiceCount
    ) {
        const std::string_view shake =
            dice.substr(
                offset,
                cryptomachine::kDiceCount
            );

        check(
            app.enter_shake_for_review(shake) ==
                cryptomachine::SeedAppStatus::Success,
            "real shake must enter review"
        );

        check(
            app.pending_shake() == shake,
            "pending review must match entered shake"
        );

        check(
            app.confirm_pending_shake() ==
                cryptomachine::SeedAppStatus::Success,
            "confirmed shake must commit successfully"
        );

        check(
            !app.has_pending_shake(),
            "confirmed shake must clear pending buffer"
        );
    }

    check(
        app.shake_count() == 10,
        "real ceremony must contain 10 confirmed shakes"
    );

    check(
        app.remaining_shakes() == 0,
        "real ceremony must have no remaining shakes"
    );

    check(
        app.ceremony_complete(),
        "real ceremony must report complete"
    );

    check(
        app.state() ==
            cryptomachine::UIState::DiceComplete,
        "last confirmed shake must enter DiceComplete"
    );

    check(
        app.sanity_report_ready(),
        "completion must create real sanity report"
    );

    const cryptomachine::FiveDiceSanity* sanity =
        app.sanity_report();

    check(
        sanity != nullptr,
        "completed ceremony must expose sanity report"
    );

    if (sanity != nullptr) {
        check(
            sanity->rolls_per_die == 10,
            "sanity report must contain 10 rolls per die"
        );

        check(
            sanity->warning_count == 0,
            "known 12-word vector must have no gross sanity warnings"
        );
    }

    check(
        app.continue_from_dice_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "completion must route using real sanity report"
    );

    check(
        app.state() ==
            cryptomachine::UIState::DiceGenerateConfirm,
        "warning-free known vector must enter generation confirmation"
    );

    check(
        app.seed_result() == nullptr,
        "mnemonic must not exist before generation confirmation"
    );

    check(
        app.generate_mnemonic() ==
            cryptomachine::SeedAppStatus::Success,
        "real ceremony generation must succeed"
    );

    check(
        app.state() ==
            cryptomachine::UIState::MnemonicWordView,
        "successful real generation must enter mnemonic view"
    );

    check(
        app.generated(),
        "controller must report generated state"
    );

    const cryptomachine::SeedResult* result =
        app.seed_result();

    check(
        result != nullptr,
        "successful generation must expose ceremony result"
    );

    if (result != nullptr) {
        check(
            mnemonic_string(*result) == expected,
            "controller must produce known 12-word mnemonic"
        );
    }

    check(
        app.mnemonic_words_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "word-by-word review must complete"
    );

    check(
        app.state() ==
            cryptomachine::UIState::MnemonicFullReview,
        "word review must advance to full mnemonic review"
    );

    check(
        app.mnemonic_review_finish() ==
            cryptomachine::SeedAppStatus::Success,
        "full review finish must request secure destruction"
    );

    check(
        app.state() ==
            cryptomachine::UIState::SessionDestroyConfirm,
        "review finish must enter destroy confirmation"
    );

    check(
        app.destroy_session() ==
            cryptomachine::SeedAppStatus::Success,
        "secure session destruction must succeed"
    );

    check(
        app.state() == cryptomachine::UIState::Home,
        "destruction must return UI Home"
    );

    check(
        app.seed_result() == nullptr,
        "destruction must remove generated result"
    );

    check(
        !app.sanity_report_ready(),
        "destruction must remove sanity report"
    );

    check(
        app.sanity_report() == nullptr,
        "destroyed sanity report must not be exposed"
    );

    check(
        !app.has_pending_shake(),
        "destruction must clear pending shake"
    );

    check(
        app.shake_count() == 0,
        "destruction must clear recorded dice history"
    );

    check(
        !app.generated(),
        "destruction must clear generated state"
    );

    check(
        !app.sensitive_data_present(),
        "destruction must clear UI sensitive-data state"
    );
}

void test_warning_route_and_restart() {
    cryptomachine::SeedAppController app;

    app.boot_complete();
    app.open_create_from_dice();
    app.choose_dice_word_count(12);
    app.begin_dice_entry();

    // D1 is permanently stuck on face 1.
    constexpr std::string_view shakes[] = {
        "11234",
        "12345",
        "13456",
        "14561",
        "15612",
        "16123",
        "11234",
        "12345",
        "13456",
        "14561",
    };

    for (std::string_view shake : shakes) {
        check(
            app.enter_shake_for_review(shake) ==
                cryptomachine::SeedAppStatus::Success,
            "warning test shake must enter review"
        );

        check(
            app.confirm_pending_shake() ==
                cryptomachine::SeedAppStatus::Success,
            "warning test shake must commit"
        );
    }

    const cryptomachine::FiveDiceSanity* sanity =
        app.sanity_report();

    check(
        sanity != nullptr,
        "warning ceremony must produce sanity report"
    );

    if (sanity != nullptr) {
        check(
            sanity->warning_count != 0,
            "stuck physical die must produce warning"
        );
    }

    check(
        app.continue_from_dice_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "warning ceremony must route successfully"
    );

    check(
        app.state() ==
            cryptomachine::UIState::DiceSanityWarning,
        "real warning must enter sanity warning screen"
    );

    check(
        app.sanity_restart() ==
            cryptomachine::SeedAppStatus::Success,
        "sanity restart must succeed"
    );

    check(
        app.state() ==
            cryptomachine::UIState::DiceIntro,
        "sanity restart must return to DiceIntro"
    );

    check(
        app.word_count() == 12,
        "sanity restart must preserve 12-word selection"
    );

    check(
        app.shake_count() == 0,
        "sanity restart must wipe old dice history"
    );

    check(
        !app.sanity_report_ready(),
        "sanity restart must wipe old sanity report"
    );

    check(
        app.sanity_report() == nullptr,
        "restarted ceremony must expose no old sanity report"
    );

    check(
        !app.generated(),
        "sanity restart must contain no generated seed"
    );

    check(
        !app.sensitive_data_present(),
        "sanity restart must clear old sensitive UI state"
    );
}

void complete_known_12_word_dice(
    cryptomachine::SeedAppController& app
) {
    constexpr std::string_view dice =
        "65515223131652132161133154444123616466443112153441";

    check(
        app.boot_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "emergency helper boot must succeed"
    );

    check(
        app.open_create_from_dice() ==
            cryptomachine::SeedAppStatus::Success,
        "emergency helper must open dice flow"
    );

    check(
        app.choose_dice_word_count(12) ==
            cryptomachine::SeedAppStatus::Success,
        "emergency helper must initialize 12-word ceremony"
    );

    check(
        app.begin_dice_entry() ==
            cryptomachine::SeedAppStatus::Success,
        "emergency helper must begin dice entry"
    );

    for (
        std::size_t offset = 0;
        offset < dice.size();
        offset += cryptomachine::kDiceCount
    ) {
        const std::string_view shake =
            dice.substr(
                offset,
                cryptomachine::kDiceCount
            );

        check(
            app.enter_shake_for_review(shake) ==
                cryptomachine::SeedAppStatus::Success,
            "emergency helper shake must enter review"
        );

        check(
            app.confirm_pending_shake() ==
                cryptomachine::SeedAppStatus::Success,
            "emergency helper shake must commit"
        );
    }
}

void test_emergency_destroy_pending_shake() {
    cryptomachine::SeedAppController app;

    app.boot_complete();
    app.open_create_from_dice();
    app.choose_dice_word_count(12);
    app.begin_dice_entry();

    check(
        app.enter_shake_for_review("12345") ==
            cryptomachine::SeedAppStatus::Success,
        "emergency test must create pending shake"
    );

    check(
        app.has_pending_shake(),
        "pending shake must exist before emergency wipe"
    );

    check(
        app.sensitive_data_present(),
        "pending-shake workflow must be sensitive"
    );

    app.emergency_destroy_session();

    check(
        app.state() == cryptomachine::UIState::Home,
        "emergency wipe must return controller Home"
    );

    check(
        app.word_count() == 0,
        "emergency wipe must clear selected word count"
    );

    check(
        !app.has_pending_shake(),
        "emergency wipe must clear pending shake"
    );

    check(
        app.pending_shake().empty(),
        "emergency wipe must expose no pending shake"
    );

    check(
        app.shake_count() == 0,
        "emergency wipe must clear dice history"
    );

    check(
        app.seed_result() == nullptr,
        "emergency wipe must expose no seed result"
    );

    check(
        !app.sanity_report_ready(),
        "emergency wipe must clear sanity state"
    );

    check(
        app.sanity_report() == nullptr,
        "emergency wipe must expose no sanity report"
    );

    check(
        !app.generated(),
        "emergency wipe must clear generated state"
    );

    check(
        !app.sensitive_data_present(),
        "emergency wipe must clear sensitive-data state"
    );
}

void test_emergency_destroy_generated_session() {
    cryptomachine::SeedAppController app;

    complete_known_12_word_dice(app);

    check(
        app.state() ==
            cryptomachine::UIState::DiceComplete,
        "emergency generated test must reach DiceComplete"
    );

    check(
        app.shake_count() == 10,
        "dice history must exist before emergency wipe"
    );

    check(
        app.sanity_report_ready(),
        "sanity report must exist before emergency wipe"
    );

    check(
        app.sanity_report() != nullptr,
        "sanity report must be accessible before emergency wipe"
    );

    check(
        app.continue_from_dice_complete() ==
            cryptomachine::SeedAppStatus::Success,
        "emergency generated test must reach generation confirmation"
    );

    check(
        app.generate_mnemonic() ==
            cryptomachine::SeedAppStatus::Success,
        "emergency generated test must generate mnemonic"
    );

    check(
        app.seed_result() != nullptr,
        "generated result must exist before emergency wipe"
    );

    check(
        app.generated(),
        "generated state must be true before emergency wipe"
    );

    app.emergency_destroy_session();

    check(
        app.state() == cryptomachine::UIState::Home,
        "generated emergency wipe must return Home"
    );

    check(
        app.word_count() == 0,
        "generated emergency wipe must clear word count"
    );

    check(
        app.shake_count() == 0,
        "generated emergency wipe must clear dice history"
    );

    check(
        !app.ceremony_complete(),
        "generated emergency wipe must clear completion state"
    );

    check(
        app.seed_result() == nullptr,
        "generated emergency wipe must remove mnemonic result"
    );

    check(
        !app.generated(),
        "generated emergency wipe must clear generated state"
    );

    check(
        !app.sanity_report_ready(),
        "generated emergency wipe must clear sanity state"
    );

    check(
        app.sanity_report() == nullptr,
        "generated emergency wipe must expose no sanity report"
    );

    check(
        !app.has_pending_shake(),
        "generated emergency wipe must clear pending shake"
    );

    check(
        !app.sensitive_data_present(),
        "generated emergency wipe must clear sensitive-data state"
    );
}

}  // namespace

int main() {
    test_real_12_word_workflow();
    test_warning_route_and_restart();
    test_emergency_destroy_pending_shake();
    test_emergency_destroy_generated_session();

    if (failures != 0) {
        std::cerr
            << failures
            << " seed app controller test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ seed app controller integration tests passed.\n";

    return 0;
}