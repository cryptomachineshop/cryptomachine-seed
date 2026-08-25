from core.word_search import search_words, resolve_word


matches = search_words("ab")

print("Words beginning with 'ab':")
print(matches)
print()

if "abandon" not in matches:
    raise SystemExit("FAIL: abandon missing from 'ab' search.")


resolved = resolve_word("aban")

print("Resolve 'aban':", resolved)

if resolved != "abandon":
    raise SystemExit("FAIL: 'aban' did not resolve to abandon.")


resolved_upper = resolve_word("ABAN")

if resolved_upper != "abandon":
    raise SystemExit("FAIL: Uppercase prefix did not normalize correctly.")


short_word = resolve_word("zoo")

print("Resolve 'zoo':", short_word)

if short_word != "zoo":
    raise SystemExit("FAIL: Short exact BIP39 word failed.")


try:
    resolve_word("ab")
except ValueError as exc:
    print("Expected ambiguous prefix:", exc)
else:
    raise SystemExit("FAIL: Ambiguous prefix 'ab' was accepted.")


try:
    resolve_word("bitcoin")
except ValueError as exc:
    print("Expected invalid word:", exc)
else:
    raise SystemExit("FAIL: Invalid BIP39 word was accepted.")


print()
print("PASS: BIP39 word search and prefix resolution tests passed.")
