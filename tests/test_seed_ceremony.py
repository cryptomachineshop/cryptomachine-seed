from core.seed_ceremony import SeedCeremony


DICE_12 = (
    "65515223131652132161133154444123616466443112153441"
)

EXPECTED_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)


ceremony = SeedCeremony(12)

print(
    f"New ceremony: {ceremony.shake_count}/{ceremony.total_shakes} shakes"
)

try:
    ceremony.generate()
except ValueError as exc:
    print("Expected early-generate rejection:", exc)
else:
    raise SystemExit(
        "FAIL: Incomplete ceremony generated a seed."
    )


for i in range(0, len(DICE_12), 5):
    shake = DICE_12[i:i + 5]
    ceremony.add_shake(shake)

print(
    f"Completed ceremony: {ceremony.shake_count}/{ceremony.total_shakes} shakes"
)

if not ceremony.complete:
    raise SystemExit("FAIL: Ceremony should be complete.")


result = ceremony.generate()

print()
print("Generated mnemonic:")
print(result["mnemonic"])
print("Warnings:", result["sanity"]["warnings"])
print()


if result["mnemonic"] != EXPECTED_12:
    raise SystemExit(
        "FAIL: Ceremony produced incorrect mnemonic."
    )


try:
    ceremony.add_shake([1, 2, 3, 4, 5])
except ValueError:
    pass
else:
    raise SystemExit(
        "FAIL: Ceremony accepted dice after generation."
    )


ceremony.destroy_session()

if ceremony.shake_count != 0:
    raise SystemExit(
        "FAIL: Destroy session did not clear dice session."
    )

if ceremony.generated:
    raise SystemExit(
        "FAIL: Destroy session did not clear generated state."
    )


print("PASS: Seed ceremony workflow tests passed.")
