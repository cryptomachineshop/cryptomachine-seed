from core.dice_session import DiceSession


session = DiceSession(12)

if session.total_shakes != 10:
    raise SystemExit("FAIL: 12-word session should require 10 shakes.")

if session.complete:
    raise SystemExit("FAIL: New session should not be complete.")


session.add_shake([6, 2, 4, 1, 5])
session.add_shake([3, 5, 1, 2, 6])

print("Shake count:", session.shake_count)
print("Remaining:", session.remaining_shakes)
print("Canonical:", session.canonical_dice_string())
print()

if session.canonical_dice_string() != "6241535126":
    raise SystemExit("FAIL: Canonical dice string incorrect.")


removed = session.undo_last_shake()

print("Undo returned:", removed)
print("Canonical after undo:", session.canonical_dice_string())
print()

if session.canonical_dice_string() != "62415":
    raise SystemExit("FAIL: Undo did not correctly update dice string.")


session.add_shake([3, 5, 1, 2, 6])

while not session.complete:
    session.add_shake([1, 2, 3, 4, 5])


if session.shake_count != 10:
    raise SystemExit("FAIL: Completed session has wrong shake count.")

if len(session.canonical_dice_string()) != 50:
    raise SystemExit("FAIL: 12-word session should contain 50 outcomes.")


try:
    session.add_shake([1, 2, 3, 4, 5])
except ValueError:
    pass
else:
    raise SystemExit("FAIL: Completed session accepted another shake.")


bad_session = DiceSession(24)

try:
    bad_session.add_shake([1, 2, 3, 4, 7])
except ValueError:
    pass
else:
    raise SystemExit("FAIL: Invalid die value was accepted.")


try:
    bad_session.add_shake([1, 2, 3])
except ValueError:
    pass
else:
    raise SystemExit("FAIL: Incomplete five-dice shake was accepted.")


bad_session.add_shake([6, 6, 5, 4, 3])
bad_session.reset()

if bad_session.shake_count != 0:
    raise SystemExit("FAIL: Reset did not clear session.")


print("PASS: Dice session controller tests passed.")
