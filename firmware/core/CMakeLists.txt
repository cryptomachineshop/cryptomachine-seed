add_library(
    cryptomachine_core
    dice_policy.cpp
    sha256.cpp
    bip39.cpp
    dice_entropy.cpp
    bip39_wordlist.cpp
    dice_sanity.cpp
    seed_engine.cpp
    secure_zero.cpp
    dice_session.cpp
    seed_ceremony.cpp
    ui_state_machine.cpp
    seed_app_controller.cpp
)

target_include_directories(
    cryptomachine_core
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(
    cryptomachine_core
    PUBLIC
        cxx_std_20
)