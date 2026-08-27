cmake_minimum_required(VERSION 3.20)

project(
    cryptomachine_seed
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(firmware/core)

add_executable(
    test_cpp_dice_policy
    firmware/host_tests/test_dice_policy.cpp
)

target_link_libraries(
    test_cpp_dice_policy
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_sha256
    firmware/host_tests/test_sha256.cpp
)

target_link_libraries(
    test_cpp_sha256
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_bip39
    firmware/host_tests/test_bip39.cpp
)

target_link_libraries(
    test_cpp_bip39
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_dice_entropy
    firmware/host_tests/test_dice_entropy.cpp
)

target_link_libraries(
    test_cpp_dice_entropy
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_seed_pipeline
    firmware/host_tests/test_seed_pipeline.cpp
)

target_link_libraries(
    test_cpp_seed_pipeline
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_dice_sanity
    firmware/host_tests/test_dice_sanity.cpp
)

target_link_libraries(
    test_cpp_dice_sanity
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_seed_engine
    firmware/host_tests/test_seed_engine.cpp
)

target_link_libraries(
    test_cpp_seed_engine
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_secure_zero
    firmware/host_tests/test_secure_zero.cpp
)

target_link_libraries(
    test_cpp_secure_zero
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_dice_session
    firmware/host_tests/test_dice_session.cpp
)

target_link_libraries(
    test_cpp_dice_session
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_seed_ceremony
    firmware/host_tests/test_seed_ceremony.cpp
)

target_link_libraries(
    test_cpp_seed_ceremony
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_ui_state_machine
    firmware/host_tests/test_ui_state_machine.cpp
)

target_link_libraries(
    test_cpp_ui_state_machine
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_ui_abuse
    firmware/host_tests/test_ui_abuse.cpp
)

target_link_libraries(
    test_cpp_ui_abuse
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_seed_app_controller
    firmware/host_tests/test_seed_app_controller.cpp
)

target_link_libraries(
    test_cpp_seed_app_controller
    PRIVATE
        cryptomachine_core
)

add_executable(
    test_cpp_inactivity_policy
    firmware/host_tests/test_inactivity_policy.cpp
)

target_link_libraries(
    test_cpp_inactivity_policy
    PRIVATE
        cryptomachine_core
)

enable_testing()

add_test(
    NAME cpp_dice_policy
    COMMAND test_cpp_dice_policy
)

add_test(
    NAME cpp_sha256
    COMMAND test_cpp_sha256
)

add_test(
    NAME cpp_bip39
    COMMAND test_cpp_bip39
)

add_test(
    NAME cpp_dice_entropy
    COMMAND test_cpp_dice_entropy
)

add_test(
    NAME cpp_seed_pipeline
    COMMAND test_cpp_seed_pipeline
)

add_test(
    NAME cpp_dice_sanity
    COMMAND test_cpp_dice_sanity
)

add_test(
    NAME cpp_seed_engine
    COMMAND test_cpp_seed_engine
)

add_test(
    NAME cpp_secure_zero
    COMMAND test_cpp_secure_zero
)

add_test(
    NAME cpp_dice_session
    COMMAND test_cpp_dice_session
)

add_test(
    NAME cpp_seed_ceremony
    COMMAND test_cpp_seed_ceremony
)

add_test(
    NAME cpp_ui_state_machine
    COMMAND test_cpp_ui_state_machine
)

add_test(
    NAME cpp_ui_abuse
    COMMAND test_cpp_ui_abuse
)

add_test(
    NAME cpp_seed_app_controller
    COMMAND test_cpp_seed_app_controller
)

add_test(
    NAME cpp_inactivity_policy
    COMMAND test_cpp_inactivity_policy
)
