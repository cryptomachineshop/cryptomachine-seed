#pragma once

#include "dice_session.h"
#include "seed_engine.h"

#include <cstddef>
#include <string_view>

namespace cryptomachine {

enum class SeedCeremonyStatus {
    Success = 0,
    UnsupportedWordCount,
    NotInitialized,
    AlreadyGenerated,
    IncompleteDice,
    SessionComplete,
    InvalidShakeSize,
    InvalidDiceValue,
    NothingToUndo,
    SeedGenerationFailed,
};

class SeedCeremony {
public:
    SeedCeremony() = default;

    ~SeedCeremony();

    SeedCeremony(const SeedCeremony&) = delete;
    SeedCeremony& operator=(const SeedCeremony&) = delete;

    SeedCeremony(SeedCeremony&&) = delete;
    SeedCeremony& operator=(SeedCeremony&&) = delete;

    SeedCeremonyStatus initialize(
        std::size_t word_count
    );

    SeedCeremonyStatus add_shake(
        std::string_view results
    );

    SeedCeremonyStatus undo_last_shake();

    SeedCeremonyStatus generate();

    // Wipe both the generated seed result and all
    // recorded dice outcomes. The selected 12/24-word
    // mode remains active for a fresh ceremony.
    void destroy_session();

    std::size_t word_count() const;

    std::size_t shake_count() const;

    std::size_t total_shakes() const;

    std::size_t remaining_shakes() const;

    bool complete() const;

    bool generated() const;

    // Non-owning view of the current canonical dice input.
    std::string_view canonical_dice() const;

    // Returns nullptr until a seed has been generated.
    //
    // The returned result remains owned by this ceremony
    // and becomes invalid when destroy_session(),
    // initialize(), or the destructor is called.
    const SeedResult* result() const;

private:
    DiceSession session_;
    SeedResult result_{};

    std::size_t word_count_ = 0;
    bool generated_ = false;
};

}  // namespace cryptomachine