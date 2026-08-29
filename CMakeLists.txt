cmake_minimum_required(VERSION 3.20)

project(
    cryptomachine_seed
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    CRYPTOMACHINE_HOST_SANITIZERS
    "Enable AddressSanitizer and UndefinedBehaviorSanitizer for host tests"
    OFF
)

if(CRYPTOMACHINE_HOST_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(
            FATAL_ERROR
            "Host sanitizers require Clang or GNU C++"
        )
    endif()

    if(WIN32)
        # Clang ASan on Windows must use the non-debug MSVC runtime.
        # The debug CRT can report allocator mismatches before test
        # code begins executing.
        set(
            CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreadedDLL"
        )

        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            message(
                FATAL_ERROR
                "Windows host sanitizers currently require Clang"
            )
        endif()

        if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
            message(
                FATAL_ERROR
                "Windows host sanitizer runtime handling currently supports x64 only"
            )
        endif()

        execute_process(
            COMMAND
                "${CMAKE_CXX_COMPILER}"
                -print-resource-dir
            OUTPUT_VARIABLE
                CRYPTOMACHINE_CLANG_RESOURCE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE
                CRYPTOMACHINE_CLANG_RESOURCE_RESULT
        )

        if(
            NOT CRYPTOMACHINE_CLANG_RESOURCE_RESULT EQUAL 0 OR
            CRYPTOMACHINE_CLANG_RESOURCE_DIR STREQUAL ""
        )
            message(
                FATAL_ERROR
                "Could not locate the Clang resource directory"
            )
        endif()

        set(
            CRYPTOMACHINE_ASAN_RUNTIME_DLL
            "${CRYPTOMACHINE_CLANG_RESOURCE_DIR}/lib/windows/clang_rt.asan_dynamic-x86_64.dll"
        )

        if(NOT EXISTS "${CRYPTOMACHINE_ASAN_RUNTIME_DLL}")
            message(
                FATAL_ERROR
                "Could not locate the Windows Clang ASan runtime DLL: ${CRYPTOMACHINE_ASAN_RUNTIME_DLL}"
            )
        endif()

        message(
            STATUS
            "CryptoMachine ASan runtime: ${CRYPTOMACHINE_ASAN_RUNTIME_DLL}"
        )
    endif()

    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )

    add_link_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )

    message(
        STATUS
        "CryptoMachine host sanitizers: ENABLED (ASan + UBSan)"
    )
else()
    message(
        STATUS
        "CryptoMachine host sanitizers: DISABLED"
    )
endif()

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

if(CRYPTOMACHINE_HOST_SANITIZERS AND WIN32)
    set(
        CRYPTOMACHINE_HOST_TEST_TARGETS
        test_cpp_dice_policy
        test_cpp_sha256
        test_cpp_bip39
        test_cpp_dice_entropy
        test_cpp_seed_pipeline
        test_cpp_dice_sanity
        test_cpp_seed_engine
        test_cpp_secure_zero
        test_cpp_dice_session
        test_cpp_seed_ceremony
        test_cpp_ui_state_machine
        test_cpp_ui_abuse
        test_cpp_seed_app_controller
        test_cpp_inactivity_policy
    )

    foreach(
        CRYPTOMACHINE_HOST_TEST_TARGET
        IN LISTS CRYPTOMACHINE_HOST_TEST_TARGETS
    )
        add_custom_command(
            TARGET
                ${CRYPTOMACHINE_HOST_TEST_TARGET}
            POST_BUILD
            COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_if_different
                "${CRYPTOMACHINE_ASAN_RUNTIME_DLL}"
                "$<TARGET_FILE_DIR:${CRYPTOMACHINE_HOST_TEST_TARGET}>"
            VERBATIM
        )
    endforeach()
endif()
