cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED PICO_SDK_PATH)
    message(
        FATAL_ERROR
        "PICO_SDK_PATH must point to the Raspberry Pi Pico SDK"
    )
endif()

set(
    PICO_BOARD
    cryptomachine_rp2350_touch_lcd_3_5
    CACHE STRING
    "CryptoMachine RP2350 Touch LCD 3.5 board"
)

list(
    APPEND
    PICO_BOARD_HEADER_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/boards
)

include(
    ${PICO_SDK_PATH}/external/pico_sdk_import.cmake
)

project(
    cryptomachine_seed_rp2350
    VERSION 0.1.0
    LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

pico_sdk_init()

add_subdirectory(
    ../core
    ${CMAKE_CURRENT_BINARY_DIR}/cryptomachine_core
)

add_subdirectory(third_party/lvgl)

add_executable(
    cryptomachine_seed
    main.cpp
    hardware/board_init.cpp
    hardware/display_st7796.cpp
    hardware/touch_ft6336u.cpp
    ui/lvgl_port.cpp
    ui/bringup_screen.cpp
)

target_include_directories(
    cryptomachine_seed
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/hardware
        ${CMAKE_CURRENT_LIST_DIR}/ui
)

target_link_libraries(
    cryptomachine_seed
    PRIVATE
        pico_stdlib
        hardware_gpio
        hardware_spi
        hardware_i2c
        hardware_pwm
        hardware_clocks
        hardware_vreg
        cryptomachine_lvgl
        cryptomachine_core
)

pico_enable_stdio_usb(
    cryptomachine_seed
    1
)

pico_enable_stdio_uart(
    cryptomachine_seed
    0
)

pico_add_extra_outputs(
    cryptomachine_seed
)