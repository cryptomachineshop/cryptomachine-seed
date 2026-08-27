#include "inactivity_policy.h"

namespace cryptomachine {
namespace {

std::uint64_t elapsed_since(
    std::uint64_t now_ms,
    std::uint64_t then_ms
) {
    // Defensive behavior if a caller ever supplies a regressed clock value.
    // The RP2350 integration uses a monotonic timer, so normal operation
    // always follows the subtraction branch.
    if (now_ms < then_ms) {
        return 0;
    }

    return now_ms - then_ms;
}

std::uint32_t warning_seconds_remaining(
    std::uint64_t elapsed_ms
) {
    if (elapsed_ms >= kInactivityWipeAfterMs) {
        return 0;
    }

    const std::uint64_t remaining_ms =
        kInactivityWipeAfterMs - elapsed_ms;

    std::uint32_t seconds =
        static_cast<std::uint32_t>(
            (
                remaining_ms +
                kInactivityMillisecondsPerSecond - 1
            ) /
            kInactivityMillisecondsPerSecond
        );

    if (seconds == 0) {
        seconds = 1;
    }

    return seconds;
}

}  // namespace

void InactivityPolicy::reset(
    std::uint64_t now_ms
) {
    last_activity_ms_ = now_ms;
    phase_ = InactivityPhase::Active;
    last_warning_seconds_ = 0;
}

InactivityDecision InactivityPolicy::update(
    std::uint64_t now_ms,
    bool is_home,
    bool touch_activity
) {
    InactivityDecision decision{};

    if (is_home) {
        const bool was_inactive =
            phase_ != InactivityPhase::Active;

        reset(now_ms);

        if (was_inactive) {
            decision.event =
                InactivityEvent::HomeReset;
        }

        decision.phase = phase_;
        return decision;
    }

    if (touch_activity) {
        if (phase_ == InactivityPhase::Warning) {
            decision.event =
                InactivityEvent::WakeFromWarning;
        } else if (
            phase_ == InactivityPhase::Dimmed
        ) {
            decision.event =
                InactivityEvent::WakeFromDim;
        }

        reset(now_ms);

        decision.phase = phase_;
        return decision;
    }

    const std::uint64_t elapsed_ms =
        elapsed_since(
            now_ms,
            last_activity_ms_
        );

    if (elapsed_ms >= kInactivityWipeAfterMs) {
        // Make expiry one-shot even if the caller needs another loop
        // iteration to finish the concrete destruction action.
        reset(now_ms);

        decision.event =
            InactivityEvent::Expire;
        decision.phase = phase_;
        return decision;
    }

    if (elapsed_ms >= kInactivityWarningAfterMs) {
        const std::uint32_t seconds =
            warning_seconds_remaining(
                elapsed_ms
            );

        if (phase_ != InactivityPhase::Warning) {
            phase_ = InactivityPhase::Warning;
            last_warning_seconds_ = seconds;

            decision.event =
                InactivityEvent::EnterWarning;
            decision.warning_seconds =
                seconds;
        } else if (
            seconds != last_warning_seconds_
        ) {
            last_warning_seconds_ = seconds;

            decision.event =
                InactivityEvent::WarningTick;
            decision.warning_seconds =
                seconds;
        }

        decision.phase = phase_;
        return decision;
    }

    if (
        elapsed_ms >= kInactivityDimAfterMs &&
        phase_ == InactivityPhase::Active
    ) {
        phase_ = InactivityPhase::Dimmed;

        decision.event =
            InactivityEvent::EnterDim;
    }

    decision.phase = phase_;
    return decision;
}

InactivityPhase InactivityPolicy::phase() const {
    return phase_;
}

std::uint64_t InactivityPolicy::last_activity_ms() const {
    return last_activity_ms_;
}

}  // namespace cryptomachine
