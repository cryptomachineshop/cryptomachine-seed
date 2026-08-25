from core.dice_policy import validate_outcome_count
from core.dice_sanity import analyze_five_dice
from core.reference_engine import dice_to_mnemonic


def create_seed_from_dice(dice, word_count):
    """
    Production V1 dice-to-mnemonic entry point.

    This function:

    1. Enforces the exact V1 dice outcome count.
    2. Runs aggregate and per-die sanity analysis.
    3. Generates the deterministic BIP39 mnemonic.
    4. Returns the mnemonic plus sanity information.

    Sanity warnings do not alter the cryptographic result.
    They are informational warnings for the user.
    """

    validate_outcome_count(
        dice,
        word_count,
    )

    sanity = analyze_five_dice(dice)

    mnemonic = dice_to_mnemonic(
        dice,
        word_count,
    )

    return {
        "mnemonic": mnemonic,
        "sanity": sanity,
    }
