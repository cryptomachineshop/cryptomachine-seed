from core.dice_sanity import analyze_dice


NORMAL = (
    "123456"
    "615243"
    "362514"
    "451326"
    "246135"
    "531642"
    "164253"
    "325416"
    "642531"
)

MISSING_FACE = "12345123451234512345123451234512345123451234512345"

LONG_REPEAT = "123456" + ("4" * 9) + "123456123456123456"

GROSSLY_BIASED = ("1" * 40) + "23456234562345623456"


normal = analyze_dice(NORMAL)

print("Normal counts:", normal["counts"])
print("Normal warnings:", normal["warnings"])
print()

if normal["warnings"]:
    raise SystemExit("FAIL: Normal test sequence produced a warning.")


missing = analyze_dice(MISSING_FACE)

print("Missing-face warnings:")
for warning in missing["warnings"]:
    print(" -", warning)
print()

if not any("never appeared" in w for w in missing["warnings"]):
    raise SystemExit("FAIL: Missing face was not detected.")


repeat = analyze_dice(LONG_REPEAT)

print("Repeated-run warnings:")
for warning in repeat["warnings"]:
    print(" -", warning)
print()

if repeat["longest_run"] < 8:
    raise SystemExit("FAIL: Long repeated run was not detected.")


biased = analyze_dice(GROSSLY_BIASED)

print("Gross-bias warnings:")
for warning in biased["warnings"]:
    print(" -", warning)
print()

if not any("much more often" in w for w in biased["warnings"]):
    raise SystemExit("FAIL: Gross imbalance was not detected.")


print("PASS: Dice sanity-check tests passed.")
