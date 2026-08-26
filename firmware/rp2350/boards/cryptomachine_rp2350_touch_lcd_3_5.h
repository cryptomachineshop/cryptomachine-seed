#ifndef _BOARDS_CRYPTOMACHINE_RP2350_TOUCH_LCD_3_5_H
#define _BOARDS_CRYPTOMACHINE_RP2350_TOUCH_LCD_3_5_H

pico_board_cmake_set(PICO_PLATFORM, rp2350)

#define CRYPTOMACHINE_RP2350_TOUCH_LCD_3_5

// RP2350B, 48-GPIO package.
#define PICO_RP2350A 0

// 16 MB external flash.
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

pico_board_cmake_set_default(
    PICO_FLASH_SIZE_BYTES,
    (16 * 1024 * 1024)
)

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

pico_board_cmake_set_default(
    PICO_RP2350_A2_SUPPORTED,
    1
)

#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

#endif