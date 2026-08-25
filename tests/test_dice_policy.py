from core.dice_policy import (
    required_outcomes,
    required_shakes,
    validate_outcome_count,
)


if required_outcomes(12) != 50:
    raise SystemExit("FAIL: 12-word outcome count.")

if required_outcomes(24) != 100:
    raise SystemExit("FAIL: 24-word outcome count.")

if required_shakes(12) != 10:
    raise SystemExit("FAIL: 12-word shake count.")

if required_shakes(24) != 20:
    raise SystemExit("FAIL: 24-word shake count.")


validate_outcome_count("1" * 50, 12)
validate_outcome_count("1" * 100, 24)


try:
    validate_outcome_count("1" * 99, 24)
except ValueError:
    pass
else:
    raise SystemExit("FAIL: 99 outcomes should be rejected in production 24-word mode.")


print("12-word mode: 10 shakes / 50 outcomes")
print("24-word mode: 20 shakes / 100 outcomes")
print()
print("PASS: Dice policy tests passed.")
