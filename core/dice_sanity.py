from collections import Counter


def analyze_dice(dice):
    """
    Perform simple sanity checks on a dice string.

    This does NOT prove that the dice are fair or that the input
    contains a specific amount of entropy.

    It only looks for obvious suspicious patterns.

    Returns:
        {
            "counts": {...},
            "warnings": [...]
        }
    """

    if not dice:
        raise ValueError("Dice input cannot be empty.")

    if any(char not in "123456" for char in dice):
        raise ValueError("Dice input may contain only digits 1 through 6.")

    counts = Counter(dice)
    warnings = []

    total = len(dice)

    # Missing faces
    missing = [
        face for face in "123456"
        if counts.get(face, 0) == 0
    ]

    if missing:
        warnings.append(
            "One or more die faces never appeared: "
            + ", ".join(missing)
        )

    # Very long identical runs
    longest_run = 1
    current_run = 1

    for i in range(1, total):
        if dice[i] == dice[i - 1]:
            current_run += 1
            longest_run = max(longest_run, current_run)
        else:
            current_run = 1

    if longest_run >= 8:
        warnings.append(
            f"An unusually long repeated run was detected "
            f"({longest_run} identical outcomes in a row)."
        )

    # Gross imbalance only.
    # This intentionally uses a generous threshold because this
    # feature is not a statistical certification system.
    expected = total / 6

    if total >= 50:
        for face in "123456":
            count = counts.get(face, 0)

            if count > expected * 2:
                warnings.append(
                    f"Face {face} appeared much more often than expected "
                    f"({count} of {total} outcomes)."
                )

    return {
        "counts": {
            face: counts.get(face, 0)
            for face in "123456"
        },
        "longest_run": longest_run,
        "warnings": warnings,
    }
