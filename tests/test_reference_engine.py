from core.reference_engine import dice_to_mnemonic


DICE_12 = (
    "65515223131652132161133154444123616466443112153441"
)

EXPECTED_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)


DICE_24 = (
    "655152231316521321611331544441236164664431121534415633526456254462245546236542364246312613322234612"
)

EXPECTED_24 = (
    "eyebrow obvious such suggest poet seven breeze blame virtual frown "
    "dynamic donor harsh pigeon express broccoli easy apology scatter "
    "force recipe shadow claim radio"
)


actual_12 = dice_to_mnemonic(DICE_12, 12)
actual_24 = dice_to_mnemonic(DICE_24, 24)

print("12-word result:")
print(actual_12)
print()

print("24-word result:")
print(actual_24)
print()

if actual_12 != EXPECTED_12:
    raise SystemExit("FAIL: 12-word reference test failed.")

if actual_24 != EXPECTED_24:
    raise SystemExit("FAIL: 24-word reference test failed.")

print("PASS: Reusable reference engine matches both known vectors.")
