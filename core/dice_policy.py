DICE_COUNT = 5

WORD_COUNT_12 = 12
WORD_COUNT_24 = 24

OUTCOMES_12 = 50
OUTCOMES_24 = 100

SHAKES_12 = OUTCOMES_12 // DICE_COUNT
SHAKES_24 = OUTCOMES_24 // DICE_COUNT


def required_outcomes(word_count):
    if word_count == WORD_COUNT_12:
        return OUTCOMES_12

    if word_count == WORD_COUNT_24:
        return OUTCOMES_24

    raise ValueError("word_count must be 12 or 24.")


def required_shakes(word_count):
    if word_count == WORD_COUNT_12:
        return SHAKES_12

    if word_count == WORD_COUNT_24:
        return SHAKES_24

    raise ValueError("word_count must be 12 or 24.")


def validate_outcome_count(dice, word_count):
    expected = required_outcomes(word_count)

    if len(dice) != expected:
        raise ValueError(
            f"{word_count}-word mode requires exactly "
            f"{expected} dice outcomes; received {len(dice)}."
        )

    return True
