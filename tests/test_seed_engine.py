from core.seed_engine import create_seed_from_dice


DICE_12 = (
    "65515223131652132161133154444123616466443112153441"
)

EXPECTED_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)


# 100-outcome production test.
# This is deliberately deterministic.
DICE_24 = (
    "12345612345612345612345612345612345612345612345612"
    "34561234561234561234561234561234561234561234561234"
)

EXPECTED_24 = (
    "tornado cactus wheel picture target finish home neither trend picture "
    "shoulder endless deputy glide open oxygen another ability forum swear "
    "side alcohol devote random"
)

if len(DICE_24) != 100:
    raise SystemExit(
        f"FAIL: Production 24-word test vector has {len(DICE_24)} outcomes."
    )

if len(DICE_24) != 100:
    raise SystemExit(
        f"FAIL: Production 24-word test vector has {len(DICE_24)} outcomes."
    )


result_12 = create_seed_from_dice(
    DICE_12,
    12,
)

print("12-word production mnemonic:")
print(result_12["mnemonic"])
print("Warnings:", result_12["sanity"]["warnings"])
print()

if result_12["mnemonic"] != EXPECTED_12:
    raise SystemExit(
        "FAIL: Production 12-word mnemonic changed."
    )


result_24 = create_seed_from_dice(
    DICE_24,
    24,
)

print("24-word production mnemonic:")
print(result_24["mnemonic"])
print("Warnings:", result_24["sanity"]["warnings"])
print()

if result_24["mnemonic"] != EXPECTED_24:
    raise SystemExit(
        "FAIL: Production 24-word mnemonic changed."
    )


try:
    create_seed_from_dice("1" * 49, 12)
except ValueError:
    pass
else:
    raise SystemExit(
        "FAIL: 49 outcomes were accepted for 12-word mode."
    )


try:
    create_seed_from_dice("1" * 99, 24)
except ValueError:
    pass
else:
    raise SystemExit(
        "FAIL: 99 outcomes were accepted for production 24-word mode."
    )


print("PASS: Production seed engine policy tests passed.")
