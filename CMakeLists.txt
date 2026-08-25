cmake_minimum_required(VERSION 3.20)

project(
    cryptomachine_seed
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    cryptomachine_core
    firmware/core/dice_policy.cpp
    firmware/core/sha256.cpp
)

target_include_directories(
    cryptomachine_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/firmware/core
)

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

enable_testing()

add_test(
    NAME cpp_dice_policy
    COMMAND test_cpp_dice_policy
)

add_test(
    NAME cpp_sha256
    COMMAND test_cpp_sha256
)
