#pragma once

#include "dice_policy.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace cryptomachine {

constexpr std::size_t kMaxDiceOutcomes = kOutcomes24;

enum class DiceSessionStatus {
    Success = 0,
    UnsupportedWordCount,
    SessionComplete,
    InvalidShakeSize,
    InvalidDiceValue,
    NothingToUndo,
};

class DiceSession {
public:
    DiceSession() = default;

    ~DiceSession();

    DiceSession(const DiceSession&) = delete;
    DiceSession& operator=(const DiceSession&) = delete;

    DiceSession(DiceSession&&) = delete;
    DiceSession& operator=(DiceSession&&) = delete;

    DiceSessionStatus initialize(
        std::size_t word_count
    );

    DiceSessionStatus add_shake(
        std::string_view results
    );

    DiceSessionStatus undo_last_shake();

    void reset();

    std::size_t word_count() const;

    std::size_t total_shakes() const;

    std::size_t shake_count() const;

    std::size_t remaining_shakes() const;

    bool complete() const;

    std::string_view canonical_dice() const;

private:
    std::array<char, kMaxDiceOutcomes> dice_{};

    std::size_t word_count_ = 0;
    std::size_t total_shakes_ = 0;
    std::size_t shake_count_ = 0;
};

}  // namespace cryptomachine