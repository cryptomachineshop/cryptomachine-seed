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

option(
    CRYPTOMACHINE_DEV_LOGGING
    "Enable development-only USB stdio diagnostics"
    OFF
)

option(
    CRYPTOMACHINE_SIGN_FIRMWARE
    "Cryptographically sign the RP2350 firmware image"
    OFF
)

set(
    CRYPTOMACHINE_SIGNING_KEY
    ""
    CACHE FILEPATH
    "External secp256k1 PEM private key used to sign firmware"
)



option(
    CRYPTOMACHINE_PRODUCTION_RELEASE
    "Require hardened settings for a distributable production firmware release"
    OFF
)

option(
    CRYPTOMACHINE_STACK_USAGE_ANALYSIS
    "Generate GCC .su stack-usage reports for first-party firmware targets"
    OFF
)

if(CRYPTOMACHINE_PRODUCTION_RELEASE)
    if(CRYPTOMACHINE_DEV_LOGGING)
        message(
            FATAL_ERROR
            "Production release cannot enable development logging"
        )
    endif()

    if(NOT CRYPTOMACHINE_SIGN_FIRMWARE)
        message(
            FATAL_ERROR
            "Production release requires firmware signing"
        )
    endif()

    if(NOT CRYPTOMACHINE_STACK_USAGE_ANALYSIS)
        message(
            FATAL_ERROR
            "Production release requires first-party stack usage analysis"
        )
    endif()

    message(
        STATUS
        "CryptoMachine production release guard: ENABLED"
    )
else()
    message(
        STATUS
        "CryptoMachine production release guard: DISABLED"
    )
endif()

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
    ui/seed_ui.cpp
)

target_include_directories(
    cryptomachine_seed
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/hardware
        ${CMAKE_CURRENT_LIST_DIR}/ui
)

target_compile_definitions(
    cryptomachine_seed
    PRIVATE
        PICO_STACK_SIZE=0x1000
        PICO_USE_STACK_GUARDS=1
)

message(
    STATUS
    "CryptoMachine main stack: 4096 bytes with RP2350 stack guards"
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

if(CRYPTOMACHINE_STACK_USAGE_ANALYSIS)
    target_compile_options(
        cryptomachine_seed
        PRIVATE
            -fstack-usage
    )

    target_compile_options(
        cryptomachine_core
        PRIVATE
            -fstack-usage
    )

    message(
        STATUS
        "CryptoMachine first-party stack usage analysis: ENABLED"
    )
else()
    message(
        STATUS
        "CryptoMachine first-party stack usage analysis: DISABLED"
    )
endif()

if(CRYPTOMACHINE_DEV_LOGGING)
    target_compile_definitions(
        cryptomachine_seed
        PRIVATE
            CRYPTOMACHINE_DEV_LOGGING=1
    )

    pico_enable_stdio_usb(
        cryptomachine_seed
        1
    )

    message(
        STATUS
        "CryptoMachine development USB logging: ENABLED"
    )
else()
    target_compile_definitions(
        cryptomachine_seed
        PRIVATE
            CRYPTOMACHINE_DEV_LOGGING=0
    )

    pico_enable_stdio_usb(
        cryptomachine_seed
        0
    )

    message(
        STATUS
        "CryptoMachine development USB logging: DISABLED"
    )
endif()

pico_enable_stdio_uart(
    cryptomachine_seed
    0
)

if(CRYPTOMACHINE_SIGN_FIRMWARE)
    if(CRYPTOMACHINE_DEV_LOGGING)
        message(
            FATAL_ERROR
            "Signed production firmware cannot enable development logging"
        )
    endif()

    if(CRYPTOMACHINE_SIGNING_KEY STREQUAL "")
        message(
            FATAL_ERROR
            "CRYPTOMACHINE_SIGNING_KEY must point to an external "
            "secp256k1 PEM private key when firmware signing is enabled"
        )
    endif()

    if(NOT EXISTS "${CRYPTOMACHINE_SIGNING_KEY}")
        message(
            FATAL_ERROR
            "CryptoMachine signing key does not exist: "
            "${CRYPTOMACHINE_SIGNING_KEY}"
        )
    endif()

    # Configure picotool signing before pico_add_extra_outputs().
    # The OTP JSON contains the public-key fingerprint and related
    # provisioning rows. Generating this file does not program OTP.
    pico_set_otp_key_output_file(
        cryptomachine_seed
        "${CMAKE_CURRENT_BINARY_DIR}/cryptomachine_seed.otp.json"
    )

    pico_sign_binary(
        cryptomachine_seed
        "${CRYPTOMACHINE_SIGNING_KEY}"
    )

    message(
        STATUS
        "CryptoMachine firmware signing: ENABLED"
    )
else()
    message(
        STATUS
        "CryptoMachine firmware signing: DISABLED"
    )
endif()

pico_add_extra_outputs(
    cryptomachine_seed
)