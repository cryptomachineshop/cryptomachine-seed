from core.reference_engine import load_wordlist


def search_words(query, limit=None):
    """
    Return BIP39 English words beginning with query.

    Search is case-insensitive and ignores surrounding spaces.
    """

    query = query.strip().lower()

    if not query:
        return []

    if not query.isalpha():
        raise ValueError("Search may contain letters only.")

    words = load_wordlist()

    matches = [
        word for word in words
        if word.startswith(query)
    ]

    if limit is not None:
        if limit < 1:
            raise ValueError("limit must be at least 1.")
        matches = matches[:limit]

    return matches


def resolve_word(entry):
    """
    Resolve a complete BIP39 word or an unambiguous prefix.

    Exact words are always accepted.

    A unique prefix is accepted.

    Ambiguous or unknown prefixes raise ValueError.
    """

    entry = entry.strip().lower()

    if not entry:
        raise ValueError("Word entry cannot be empty.")

    words = load_wordlist()

    if entry in words:
        return entry

    matches = search_words(entry)

    if len(matches) == 1:
        return matches[0]

    if len(matches) == 0:
        raise ValueError(
            f"No BIP39 English word begins with: {entry}"
        )

    raise ValueError(
        f"Ambiguous BIP39 prefix '{entry}' matches {len(matches)} words."
    )
