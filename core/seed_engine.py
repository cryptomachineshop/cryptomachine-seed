from core.dice_policy import validate_outcome_count
from core.reference_engine import dice_to_mnemonic
from core.dice_sanity import analyze_dice


def create_seed_from_dice(dice, word_count):
    """
    Production CryptoMachine dice-to-mnemonic workflow.

    Enforces the official CryptoMachine dice count:
      12 words -> 50 outcomes
      24 words -> 100 outcomes

    Returns:
        {
            "mnemonic": "...",
            "sanity": {...}
        }
    """

    validate_outcome_count(dice, word_count)

    sanity = analyze_dice(dice)

    mnemonic = dice_to_mnemonic(
        dice,
        word_count,
    )

    return {
        "mnemonic": mnemonic,
        "sanity": sanity,
    }
