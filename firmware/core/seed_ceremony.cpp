#include "seed_ceremony.h"

namespace cryptomachine {
namespace {

SeedCeremonyStatus map_session_status(
    DiceSessionStatus status
) {
    switch (status) {
        case DiceSessionStatus::Success:
            return SeedCeremonyStatus::Success;

        case DiceSessionStatus::UnsupportedWordCount:
            return SeedCeremonyStatus::UnsupportedWordCount;

        case DiceSessionStatus::SessionComplete:
            return SeedCeremonyStatus::SessionComplete;

        case DiceSessionStatus::InvalidShakeSize:
            return SeedCeremonyStatus::InvalidShakeSize;

        case DiceSessionStatus::InvalidDiceValue:
            return SeedCeremonyStatus::InvalidDiceValue;

        case DiceSessionStatus::NothingToUndo:
            return SeedCeremonyStatus::NothingToUndo;
    }

    return SeedCeremonyStatus::SeedGenerationFailed;
}

}  // namespace

SeedCeremony::~SeedCeremony() {
    destroy_seed_result(result_);
    session_.reset();

    word_count_ = 0;
    generated_ = false;
}

SeedCeremonyStatus SeedCeremony::initialize(
    std::size_t word_count
) {
    // Destroy any prior generated result before starting
    // or reconfiguring a ceremony.
    destroy_seed_result(result_);

    generated_ = false;
    word_count_ = 0;

    const DiceSessionStatus session_status =
        session_.initialize(word_count);

    if (session_status != DiceSessionStatus::Success) {
        return map_session_status(session_status);
    }

    word_count_ = word_count;

    return SeedCeremonyStatus::Success;
}

SeedCeremonyStatus SeedCeremony::add_shake(
    std::string_view results
) {
    if (word_count_ == 0) {
        return SeedCeremonyStatus::NotInitialized;
    }

    if (generated_) {
        return SeedCeremonyStatus::AlreadyGenerated;
    }

    return map_session_status(
        session_.add_shake(results)
    );
}

SeedCeremonyStatus SeedCeremony::undo_last_shake() {
    if (word_count_ == 0) {
        return SeedCeremonyStatus::NotInitialized;
    }

    if (generated_) {
        return SeedCeremonyStatus::AlreadyGenerated;
    }

    return map_session_status(
        session_.undo_last_shake()
    );
}

SeedCeremonyStatus SeedCeremony::generate() {
    if (word_count_ == 0) {
        return SeedCeremonyStatus::NotInitialized;
    }

    if (generated_) {
        return SeedCeremonyStatus::AlreadyGenerated;
    }

    if (!session_.complete()) {
        return SeedCeremonyStatus::IncompleteDice;
    }

    const SeedEngineStatus engine_status =
        create_seed_from_dice(
            session_.canonical_dice(),
            word_count_,
            result_
        );

    if (engine_status != SeedEngineStatus::Success) {
        destroy_seed_result(result_);

        return SeedCeremonyStatus::SeedGenerationFailed;
    }

    generated_ = true;

    return SeedCeremonyStatus::Success;
}

void SeedCeremony::destroy_session() {
    destroy_seed_result(result_);

    session_.reset();

    generated_ = false;
}

std::size_t SeedCeremony::word_count() const {
    return word_count_;
}

std::size_t SeedCeremony::shake_count() const {
    return session_.shake_count();
}

std::size_t SeedCeremony::total_shakes() const {
    return session_.total_shakes();
}

std::size_t SeedCeremony::remaining_shakes() const {
    return session_.remaining_shakes();
}

bool SeedCeremony::complete() const {
    return session_.complete();
}

bool SeedCeremony::generated() const {
    return generated_;
}

std::string_view SeedCeremony::canonical_dice() const {
    return session_.canonical_dice();
}

const SeedResult* SeedCeremony::result() const {
    if (!generated_) {
        return nullptr;
    }

    return &result_;
}

}  // namespace cryptomachine