#pragma once

#include <cstdint>

namespace cryptomachine {

constexpr std::uint64_t kInactivityMillisecondsPerSecond = 1'000;
constexpr std::uint64_t kInactivityMillisecondsPerMinute =
    60 * kInactivityMillisecondsPerSecond;

constexpr std::uint64_t kInactivityDimAfterMs =
    3 * kInactivityMillisecondsPerMinute;

constexpr std::uint64_t kInactivityWarningAfterMs =
    9 * kInactivityMillisecondsPerMinute;

constexpr std::uint64_t kInactivityWipeAfterMs =
    10 * kInactivityMillisecondsPerMinute;

enum class InactivityPhase {
    Active = 0,
    Dimmed,
    Warning,
};

enum class InactivityEvent {
    None = 0,
    EnterDim,
    EnterWarning,
    WarningTick,
    WakeFromDim,
    WakeFromWarning,
    Expire,
    HomeReset,
};

struct InactivityDecision {
    InactivityEvent event = InactivityEvent::None;
    InactivityPhase phase = InactivityPhase::Active;
    std::uint32_t warning_seconds = 0;
};

// Pure, hardware-independent inactivity policy.
//
// The policy owns only timing/state metadata. It never sees seed material,
// display objects, touch coordinates, or controller internals. The RP2350
// application layer performs the concrete dim/warning/wipe actions returned
// by update().
class InactivityPolicy {
public:
    void reset(std::uint64_t now_ms);

    InactivityDecision update(
        std::uint64_t now_ms,
        bool is_home,
        bool touch_activity
    );

    InactivityPhase phase() const;
    std::uint64_t last_activity_ms() const;

private:
    std::uint64_t last_activity_ms_ = 0;
    InactivityPhase phase_ = InactivityPhase::Active;
    std::uint32_t last_warning_seconds_ = 0;
};

}  // namespace cryptomachine
