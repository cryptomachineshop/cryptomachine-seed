from core.dice_sanity import (
    analyze_dice,
    analyze_five_dice,
)


def test_balanced_aggregate():
    dice = "123456" * 9

    result = analyze_dice(dice)

    assert result["total"] == 54
    assert result["missing_faces"] == []
    assert result["longest_run"] == 1
    assert result["warnings"] == []


def test_missing_face_warning():
    dice = "12345" * 10

    result = analyze_dice(dice)

    assert "6" in result["missing_faces"]
    assert result["warnings"]


def test_long_run_warning():
    dice = "123456" * 8 + "111111111"

    result = analyze_dice(dice)

    assert result["longest_run"] >= 9
    assert result["warnings"]


def test_gross_combined_bias_warning():
    dice = "1" * 40 + "23456234562345623456"

    result = analyze_dice(dice)

    assert result["counts"]["1"] == 40
    assert result["warnings"]


def test_per_die_normal_pattern():
    shakes = [
        "12345",
        "23456",
        "34561",
        "45612",
        "56123",
        "61234",
        "12345",
        "23456",
        "34561",
        "45612",
    ]

    dice = "".join(shakes)

    result = analyze_five_dice(dice)

    assert result["rolls_per_die"] == 10
    assert result["warnings"] == []

    for die_name in (
        "D1",
        "D2",
        "D3",
        "D4",
        "D5",
    ):
        assert (
            result["per_die"][die_name]["rolls"]
            == 10
        )


def test_stuck_die_detected():
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

    result = analyze_five_dice(dice)

    d1 = result["per_die"]["D1"]

    assert d1["counts"]["6"] == 10
    assert d1["warnings"]

    assert any(
        "D1 produced the same face"
        in warning
        for warning in result["warnings"]
    )


def test_heavy_single_die_bias_detected():
    d1 = "1111111123"
    d2 = "1234561234"
    d3 = "2345612345"
    d4 = "3456123456"
    d5 = "4561234561"

    dice = "".join(
        d1[index]
        + d2[index]
        + d3[index]
        + d4[index]
        + d5[index]
        for index in range(10)
    )

    result = analyze_five_dice(dice)

    assert result["per_die"]["D1"]["counts"]["1"] == 8

    assert any(
        "D1 produced face 1 on 8 of 10 shakes."
        == warning
        for warning in result["warnings"]
    )


def test_per_die_long_run_detected():
    d1 = "1111112345"
    d2 = "1234561234"
    d3 = "2345612345"
    d4 = "3456123456"
    d5 = "4561234561"

    dice = "".join(
        d1[index]
        + d2[index]
        + d3[index]
        + d4[index]
        + d5[index]
        for index in range(10)
    )

    result = analyze_five_dice(dice)

    assert (
        result["per_die"]["D1"]["longest_run"]
        == 6
    )

    assert any(
        "D1 produced the same face for 6 "
        "consecutive shakes."
        == warning
        for warning in result["warnings"]
    )


def test_incomplete_shake_rejected():
    try:
        analyze_five_dice("123456")

    except ValueError:
        pass

    else:
        raise AssertionError(
            "Incomplete five-dice stream "
            "was not rejected"
        )


def main():
    tests = [
        test_balanced_aggregate,
        test_missing_face_warning,
        test_long_run_warning,
        test_gross_combined_bias_warning,
        test_per_die_normal_pattern,
        test_stuck_die_detected,
        test_heavy_single_die_bias_detected,
        test_per_die_long_run_detected,
        test_incomplete_shake_rejected,
    ]

    for test in tests:
        test()

    print(
        "PASS: aggregate and per-die "
        "sanity checks passed"
    )


if __name__ == "__main__":
    main()
