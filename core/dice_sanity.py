from collections import Counter
from math import ceil


VALID_FACES = "123456"
DICE_PER_SHAKE = 5


def _validate_dice_string(dice):
    if not isinstance(dice, str):
        raise ValueError("Dice input must be a string")

    if not dice:
        raise ValueError("Dice input cannot be empty")

    if any(face not in VALID_FACES for face in dice):
        raise ValueError(
            "Dice input may contain only digits 1 through 6"
        )


def _longest_identical_run(sequence):
    if not sequence:
        return 0

    longest = 1
    current = 1

    for index in range(1, len(sequence)):
        if sequence[index] == sequence[index - 1]:
            current += 1
            longest = max(longest, current)

        else:
            current = 1

    return longest


def analyze_dice(dice):
    """
    Aggregate sanity analysis.

    These checks can detect gross patterns but cannot prove
    that physical dice are fair or that entropy is sufficient.
    """

    _validate_dice_string(dice)

    total = len(dice)
    counts = Counter(dice)

    full_counts = {
        face: counts.get(face, 0)
        for face in VALID_FACES
    }

    missing_faces = [
        face
        for face in VALID_FACES
        if full_counts[face] == 0
    ]

    longest_run = _longest_identical_run(dice)

    warnings = []

    if missing_faces:
        warnings.append(
            "One or more die faces never appeared "
            "in the combined results."
        )

    if longest_run >= 8:
        warnings.append(
            "An unusually long repeated-face run "
            "was detected in the combined results."
        )

    if total >= 50:
        expected = total / 6

        for face in VALID_FACES:
            if full_counts[face] > expected * 2:
                warnings.append(
                    f"Face {face} appeared unusually often "
                    "in the combined results."
                )
                break

    return {
        "total": total,
        "counts": full_counts,
        "missing_faces": missing_faces,
        "longest_run": longest_run,
        "warnings": warnings,
    }


def analyze_five_dice(dice):
    """
    Analyze the canonical D1-D5 dice stream both in aggregate
    and by physical die position.

    Canonical input order must be:

        D1 D2 D3 D4 D5
        D1 D2 D3 D4 D5
        ...

    Per-die checks are deliberately conservative because V1
    records only 10 rolls per die for a 12-word ceremony and
    20 rolls per die for a 24-word ceremony.

    This analysis cannot prove that dice are fair or random.
    """

    _validate_dice_string(dice)

    if len(dice) % DICE_PER_SHAKE != 0:
        raise ValueError(
            "Five-dice analysis requires a complete "
            "multiple of five outcomes"
        )

    aggregate = analyze_dice(dice)

    rolls_per_die = len(dice) // DICE_PER_SHAKE

    per_die = {}
    combined_warnings = list(aggregate["warnings"])

    for die_index in range(DICE_PER_SHAKE):
        die_name = f"D{die_index + 1}"

        sequence = dice[
            die_index::DICE_PER_SHAKE
        ]

        counts = Counter(sequence)

        full_counts = {
            face: counts.get(face, 0)
            for face in VALID_FACES
        }

        longest_run = _longest_identical_run(sequence)

        die_warnings = []

        # A completely fixed die is clearly suspicious.
        if len(set(sequence)) == 1:
            die_warnings.append(
                f"{die_name} produced the same face "
                "on every recorded shake."
            )

        # Warn only on very heavy concentration.
        # 10 rolls -> threshold 8
        # 20 rolls -> threshold 15
        concentration_threshold = ceil(
            rolls_per_die * 0.75
        )

        most_common_face, most_common_count = (
            counts.most_common(1)[0]
        )

        if (
            rolls_per_die >= 10
            and most_common_count
            >= concentration_threshold
            and len(set(sequence)) > 1
        ):
            die_warnings.append(
                f"{die_name} produced face "
                f"{most_common_face} on "
                f"{most_common_count} of "
                f"{rolls_per_die} shakes."
            )

        # Six identical results in succession for one
        # physical die deserves review.
        if (
            longest_run >= 6
            and len(set(sequence)) > 1
        ):
            die_warnings.append(
                f"{die_name} produced the same face "
                f"for {longest_run} consecutive shakes."
            )

        per_die[die_name] = {
            "rolls": rolls_per_die,
            "sequence": sequence,
            "counts": full_counts,
            "longest_run": longest_run,
            "warnings": die_warnings,
        }

        combined_warnings.extend(die_warnings)

    return {
        "aggregate": aggregate,
        "rolls_per_die": rolls_per_die,
        "per_die": per_die,
        "warnings": combined_warnings,
    }
