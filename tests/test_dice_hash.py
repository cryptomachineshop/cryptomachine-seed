import hashlib

dice = "655152231316521321611331544441236164664431121534415633526456254462245546236542364246312613322234612"

expected_sha256 = "51531761ec7a738946e0b9f46bb11320a695495430e345c14f01ad8b3b898a6d"

digest = hashlib.sha256(dice.encode("ascii")).hexdigest()

print("Dice outcomes:", len(dice))
print("SHA-256:", digest)

if digest == expected_sha256:
    print("PASS: SHA-256 output matches expected value.")
else:
    print("FAIL: SHA-256 output does not match expected value.")
    raise SystemExit(1)
