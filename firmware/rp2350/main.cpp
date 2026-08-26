#include "pico/stdlib.h"

#include <cstdint>
#include <cstdio>

int main() {
    stdio_init_all();

    // Give USB CDC a moment to enumerate after boot.
    sleep_ms(1500);

    std::printf(
        "\nCryptoMachine Seed RP2350 bring-up\n"
    );

    std::printf(
        "RP2350B firmware is running.\n"
    );

    std::uint32_t heartbeat = 0;

    while (true) {
        std::printf(
            "heartbeat %lu\n",
            static_cast<unsigned long>(
                heartbeat++
            )
        );

        sleep_ms(2000);
    }
}