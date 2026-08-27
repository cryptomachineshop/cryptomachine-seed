#include "inactivity_policy.h"

#include <cstdint>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr
            << "FAIL: "
            << message
            << '\n';

        ++failures;
    }
}

void test_exact_policy_constants() {
    check(
        cryptomachine::kInactivityDimAfterMs ==
            180'000,
        "dim threshold must be 3 minutes"
    );

    check(
        cryptomachine::kInactivityWarningAfterMs ==
            540'000,
        "warning threshold must be 9 minutes"
    );

    check(
        cryptomachine::kInactivityWipeAfterMs ==
            600'000,
        "wipe threshold must be 10 minutes"
    );
}

void test_dim_boundary() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(1'000);

    auto decision =
        policy.update(
            180'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "must not dim one millisecond early"
    );

    decision =
        policy.update(
            181'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "must dim exactly at 3 minutes"
    );

    check(
        decision.phase ==
            cryptomachine::InactivityPhase::Dimmed,
        "dim event must enter dimmed phase"
    );

    decision =
        policy.update(
            181'001,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "dim transition must be one-shot"
    );
}

void test_warning_boundary_and_countdown() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(0);

    auto decision =
        policy.update(
            539'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "policy must already be dimmed before warning"
    );

    decision =
        policy.update(
            540'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterWarning,
        "must enter warning exactly at 9 minutes"
    );

    check(
        decision.phase ==
            cryptomachine::InactivityPhase::Warning,
        "warning event must enter warning phase"
    );

    check(
        decision.warning_seconds == 60,
        "warning must begin at 60 seconds"
    );

    decision =
        policy.update(
            540'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "countdown must not update before displayed second changes"
    );

    decision =
        policy.update(
            541'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::WarningTick,
        "countdown must tick after one second"
    );

    check(
        decision.warning_seconds == 59,
        "countdown must show 59 seconds after one second"
    );

    decision =
        policy.update(
            599'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::WarningTick,
        "last pre-expiry update must tick countdown"
    );

    check(
        decision.warning_seconds == 1,
        "last pre-expiry countdown must show 1 second"
    );
}

void test_expiry_boundary_is_one_shot() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(0);

    auto decision =
        policy.update(
            600'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::Expire,
        "must expire exactly at 10 minutes"
    );

    check(
        decision.phase ==
            cryptomachine::InactivityPhase::Active,
        "expiry must internally reset policy phase"
    );

    decision =
        policy.update(
            600'001,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "expiry must not repeat on next loop iteration"
    );
}

void test_touch_wakes_dim_and_restarts_clock() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(10'000);

    auto decision =
        policy.update(
            190'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "setup must enter dim phase"
    );

    decision =
        policy.update(
            200'000,
            false,
            true
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::WakeFromDim,
        "touch must wake from dim"
    );

    check(
        decision.phase ==
            cryptomachine::InactivityPhase::Active,
        "dim wake must restore active phase"
    );

    decision =
        policy.update(
            379'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "wake must restart the full dim timer"
    );

    decision =
        policy.update(
            380'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "dim must recur 3 minutes after wake"
    );
}

void test_touch_wakes_warning_and_restarts_clock() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(0);

    auto decision =
        policy.update(
            540'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterWarning,
        "setup must enter warning phase"
    );

    decision =
        policy.update(
            550'000,
            false,
            true
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::WakeFromWarning,
        "touch must wake from warning"
    );

    check(
        policy.last_activity_ms() == 550'000,
        "warning wake must become new activity origin"
    );

    decision =
        policy.update(
            729'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "warning wake must restart dim timer"
    );

    decision =
        policy.update(
            730'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "post-warning wake must dim after a fresh 3 minutes"
    );
}

void test_touch_while_active_restarts_clock() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(0);

    auto decision =
        policy.update(
            100'000,
            false,
            true
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "active touch needs no UI transition"
    );

    check(
        policy.last_activity_ms() == 100'000,
        "active touch must reset activity timestamp"
    );

    decision =
        policy.update(
            279'999,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "active touch must restart dim interval"
    );

    decision =
        policy.update(
            280'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterDim,
        "dim threshold must be relative to latest touch"
    );
}

void test_home_bypasses_and_resets_inactivity() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(0);

    auto decision =
        policy.update(
            540'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::EnterWarning,
        "setup must enter warning"
    );

    decision =
        policy.update(
            550'000,
            true,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::HomeReset,
        "Home must cancel an inactive ceremony phase"
    );

    check(
        decision.phase ==
            cryptomachine::InactivityPhase::Active,
        "Home reset must restore active phase"
    );

    decision =
        policy.update(
            5'000'000,
            true,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "remaining on Home must never expire"
    );

    check(
        policy.last_activity_ms() == 5'000'000,
        "Home must continuously restart inactivity clock"
    );
}

void test_regressed_clock_cannot_force_expiry() {
    cryptomachine::InactivityPolicy policy;
    policy.reset(1'000'000);

    const auto decision =
        policy.update(
            999'000,
            false,
            false
        );

    check(
        decision.event ==
            cryptomachine::InactivityEvent::None,
        "clock regression must not underflow into expiry"
    );
}

}  // namespace

int main() {
    test_exact_policy_constants();
    test_dim_boundary();
    test_warning_boundary_and_countdown();
    test_expiry_boundary_is_one_shot();
    test_touch_wakes_dim_and_restarts_clock();
    test_touch_wakes_warning_and_restarts_clock();
    test_touch_while_active_restarts_clock();
    test_home_bypasses_and_resets_inactivity();
    test_regressed_clock_cannot_force_expiry();

    if (failures != 0) {
        std::cerr
            << failures
            << " inactivity policy test(s) failed.\n";

        return 1;
    }

    std::cout
        << "All C++ inactivity policy tests passed.\n";

    return 0;
}
