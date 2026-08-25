#include "dice_policy.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace cryptomachine;


int main() {
    assert(required_outcomes(12) == 50);
    assert(required_outcomes(24) == 100);

    assert(required_shakes(12) == 10);
    assert(required_shakes(24) == 20);

    assert(valid_dice_char('1'));
    assert(valid_dice_char('6'));

    assert(!valid_dice_char('0'));
    assert(!valid_dice_char('7'));
    assert(!valid_dice_char('x'));

    const std::string dice_12(
        50,
        '1'
    );

    const std::string dice_24(
        100,
        '6'
    );

    assert(
        validate_outcome_count(
            dice_12,
            12
        )
    );

    assert(
        validate_outcome_count(
            dice_24,
            24
        )
    );

    assert(
        !validate_outcome_count(
            dice_12.substr(0, 49),
            12
        )
    );

    assert(
        !validate_outcome_count(
            dice_24.substr(0, 99),
            24
        )
    );

    std::string invalid = dice_12;
    invalid[25] = '7';

    assert(
        !validate_outcome_count(
            invalid,
            12
        )
    );

    bool rejected = false;

    try {
        required_outcomes(18);
    }
    catch (const std::invalid_argument&) {
        rejected = true;
    }

    assert(rejected);

    std::cout
        << "PASS: C++ V1 dice policy tests"
        << std::endl;

    return 0;
}
