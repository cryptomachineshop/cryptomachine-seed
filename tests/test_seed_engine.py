from core.seed_engine import create_seed_from_dice


DICE_12 = (
    "65515223131652132161133154444123616466443112153441"
)

EXPECTED_12 = (
    "hole luggage safe present express tragic orbit shed "
    "switch metal identify path"
)


DICE_24 = (
    "12345612345612345612345612345612345612345612345612"
    "34561234561234561234561234561234561234561234561234"
)

EXPECTED_24 = (
    "tornado cactus wheel picture target finish home neither "
    "trend picture shoulder endless deputy glide open oxygen "
    "another ability forum swear side alcohol devote random"
)


def test_12_word_seed():
    result = create_seed_from_dice(
        DICE_12,
        12,
    )

    assert result["mnemonic"] == EXPECTED_12

    sanity = result["sanity"]

    assert sanity["rolls_per_die"] == 10
    assert "aggregate" in sanity
    assert "per_die" in sanity

    for die_name in (
        "D1",
        "D2",
        "D3",
        "D4",
        "D5",
    ):
        assert die_name in sanity["per_die"]


def test_24_word_seed():
    result = create_seed_from_dice(
        DICE_24,
        24,
    )

    assert result["mnemonic"] == EXPECTED_24

    sanity = result["sanity"]

    assert sanity["rolls_per_die"] == 20
    assert "aggregate" in sanity
    assert "per_die" in sanity


def test_per_die_warning_reaches_seed_engine():
    shakes = [
        "61234",
        "62345",
        "63456",
        "64561",
        "65612",
        "66123",
        "61234",
        "62345",
        "63456",
        "64561",
    ]

    dice = "".join(shakes)

    assert len(dice) == 50

    result = create_seed_from_dice(
        dice,
        12,
    )

    warnings = result["sanity"]["warnings"]

    assert any(
        "D1 produced the same face"
        in warning
        for warning in warnings
    )


def test_49_outcomes_rejected():
    try:
        create_seed_from_dice(
            DICE_12[:-1],
            12,
        )

    except ValueError:
        pass

    else:
        raise AssertionError(
            "49-outcome production input "
            "was not rejected"
        )


def test_99_outcomes_rejected():
    try:
        create_seed_from_dice(
            DICE_24[:-1],
            24,
        )

    except ValueError:
        pass

    else:
        raise AssertionError(
            "99-outcome production input "
            "was not rejected"
        )


def main():
    tests = [
        test_12_word_seed,
        test_24_word_seed,
        test_per_die_warning_reaches_seed_engine,
        test_49_outcomes_rejected,
        test_99_outcomes_rejected,
    ]

    for test in tests:
        test()

    print(
        "PASS: production seed engine and "
        "per-die sanity integration passed"
    )


if __name__ == "__main__":
    main()
