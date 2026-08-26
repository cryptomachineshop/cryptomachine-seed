#include "dice_session.h"

#include "secure_zero.h"

namespace cryptomachine {

DiceSession::~DiceSession() {
    secure_zero(
        dice_.data(),
        dice_.size()
    );

    word_count_ = 0;
    total_shakes_ = 0;
    shake_count_ = 0;
}

DiceSessionStatus DiceSession::initialize(
    std::size_t word_count
) {
    // Wipe anything left from a previous session before
    // accepting a new configuration.
    secure_zero(
        dice_.data(),
        dice_.size()
    );

    word_count_ = 0;
    total_shakes_ = 0;
    shake_count_ = 0;

    if (
        word_count != kWordCount12 &&
        word_count != kWordCount24
    ) {
        return DiceSessionStatus::UnsupportedWordCount;
    }

    word_count_ = word_count;
    total_shakes_ = required_shakes(word_count);

    return DiceSessionStatus::Success;
}

DiceSessionStatus DiceSession::add_shake(
    std::string_view results
) {
    if (
        word_count_ != kWordCount12 &&
        word_count_ != kWordCount24
    ) {
        return DiceSessionStatus::UnsupportedWordCount;
    }

    if (complete()) {
        return DiceSessionStatus::SessionComplete;
    }

    if (results.size() != kDiceCount) {
        return DiceSessionStatus::InvalidShakeSize;
    }

    for (const char value : results) {
        if (!valid_dice_char(value)) {
            return DiceSessionStatus::InvalidDiceValue;
        }
    }

    const std::size_t offset =
        shake_count_ * kDiceCount;

    for (std::size_t i = 0; i < kDiceCount; ++i) {
        dice_[offset + i] = results[i];
    }

    ++shake_count_;

    return DiceSessionStatus::Success;
}

DiceSessionStatus DiceSession::undo_last_shake() {
    if (shake_count_ == 0) {
        return DiceSessionStatus::NothingToUndo;
    }

    --shake_count_;

    const std::size_t offset =
        shake_count_ * kDiceCount;

    // The removed shake is sensitive too. Overwrite its
    // five bytes immediately instead of merely reducing
    // the logical length of the session.
    secure_zero(
        dice_.data() + offset,
        kDiceCount
    );

    return DiceSessionStatus::Success;
}

void DiceSession::reset() {
    // Preserve the selected 12/24-word mode, matching
    // the Python session behavior, but destroy all
    // recorded dice outcomes.
    secure_zero(
        dice_.data(),
        dice_.size()
    );

    shake_count_ = 0;
}

std::size_t DiceSession::word_count() const {
    return word_count_;
}

std::size_t DiceSession::total_shakes() const {
    return total_shakes_;
}

std::size_t DiceSession::shake_count() const {
    return shake_count_;
}

std::size_t DiceSession::remaining_shakes() const {
    return total_shakes_ - shake_count_;
}

bool DiceSession::complete() const {
    return (
        total_shakes_ != 0 &&
        shake_count_ == total_shakes_
    );
}

std::string_view DiceSession::canonical_dice() const {
    return std::string_view(
        dice_.data(),
        shake_count_ * kDiceCount
    );
}

}  // namespace cryptomachine