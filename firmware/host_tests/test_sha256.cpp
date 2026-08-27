#include "test_format_helpers.h"
#include "sha256.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace cryptomachine;

int main() {
    assert(
        sha256_hex(sha256(""))
        ==
        "e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855"
    );

    assert(
        sha256_hex(sha256("abc"))
        ==
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad"
    );

    const std::string dice_50 =
        "655152231316521321611331544441"
        "23616466443112153441";

    assert(dice_50.size() == 50);

    assert(
        sha256_hex(sha256(dice_50))
        ==
        "6cb09af855050dcde6fe2adc3181c250"
        "982011e2cf17821cbed56a908ec527c3"
    );

    const std::string dice_99 =
        "655152231316521321611331544441"
        "236164664431121534415633526456"
        "254462245546236542364246312613"
        "322234612";

    assert(dice_99.size() == 99);

    assert(
        sha256_hex(sha256(dice_99))
        ==
        "51531761ec7a738946e0b9f46bb11320"
        "a695495430e345c14f01ad8b3b898a6d"
    );

    const std::string dice_100 =
        "123456123456123456123456123456"
        "123456123456123456123456123456"
        "123456123456123456123456123456"
        "1234561234";

    assert(dice_100.size() == 100);

    assert(
        sha256_hex(sha256(dice_100))
        ==
        "e56403e8522ddeae1b44a1e8148b1ba4"
        "d3b4c626ccf20980056eedcc7e0c0f35"
    );

    std::cout
        << "PASS: C++ SHA-256 vectors"
        << std::endl;

    return 0;
}
