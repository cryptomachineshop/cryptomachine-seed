from core.ui_state_machine import UIState, UIStateMachine


def expect_runtime_error(action):
    try:
        action()
    except RuntimeError:
        return

    raise AssertionError(
        "Expected RuntimeError, but action was allowed"
    )


def expect_value_error(action):
    try:
        action()
    except ValueError:
        return

    raise AssertionError(
        "Expected ValueError, but action was allowed"
    )


def new_ui():
    ui = UIStateMachine()
    ui.boot_complete()
    return ui


def test_generate_from_home_rejected():
    ui = new_ui()

    expect_runtime_error(
        ui.generate_mnemonic
    )

    assert ui.state == UIState.HOME
    assert ui.mnemonic_generated is False


def test_begin_dice_without_session_rejected():
    ui = new_ui()

    expect_runtime_error(
        ui.begin_dice_entry
    )

    assert ui.state == UIState.HOME


def test_invalid_dice_word_count_rejected():
    ui = new_ui()
    ui.open_create_from_dice()

    expect_value_error(
        lambda: ui.choose_dice_word_count(18)
    )

    assert ui.state == UIState.DICE_WORD_COUNT
    assert ui.dice_session_active is False


def test_generate_too_early_rejected():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()

    expect_runtime_error(
        ui.generate_mnemonic
    )

    assert ui.state == UIState.DICE_ENTRY
    assert ui.mnemonic_generated is False


def test_destroy_from_home_rejected():
    ui = new_ui()

    expect_runtime_error(
        ui.request_destroy
    )

    assert ui.state == UIState.HOME


def test_double_destroy_request_rejected():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(12)
    ui.begin_dice_entry()

    ui.request_destroy()

    assert ui.state == UIState.SESSION_DESTROY_CONFIRM

    expect_runtime_error(
        ui.request_destroy
    )


def test_destroy_go_back_preserves_session():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(12)
    ui.begin_dice_entry()

    assert ui.sensitive_data_present is True

    ui.request_destroy()
    ui.destroy_go_back()

    assert ui.state == UIState.DICE_ENTRY
    assert ui.word_count == 12
    assert ui.dice_session_active is True
    assert ui.sensitive_data_present is True


def test_destroy_clears_dice_workflow():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()

    ui.request_destroy()
    ui.destroy_session()

    assert ui.state == UIState.HOME
    assert ui.word_count is None
    assert ui.expected_word_input is None
    assert ui.dice_session_active is False
    assert ui.mnemonic_generated is False
    assert ui.sensitive_data_present is False


def test_destroy_after_mnemonic_clears_state():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(12)
    ui.begin_dice_entry()
    ui.dice_entry_to_review()
    ui.shake_confirm_complete()
    ui.dice_complete_continue(warnings=False)
    ui.generate_mnemonic()

    assert ui.mnemonic_generated is True

    ui.mnemonic_words_complete()
    ui.mnemonic_review_finish()

    assert ui.state == UIState.SESSION_DESTROY_CONFIRM

    ui.destroy_session()

    assert ui.state == UIState.HOME
    assert ui.mnemonic_generated is False
    assert ui.dice_session_active is False
    assert ui.word_count is None
    assert ui.sensitive_data_present is False


def test_old_workflow_cannot_continue_after_destroy():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(12)
    ui.begin_dice_entry()

    ui.request_destroy()
    ui.destroy_session()

    expect_runtime_error(
        ui.dice_entry_to_review
    )

    expect_runtime_error(
        ui.generate_mnemonic
    )

    assert ui.state == UIState.HOME


def test_cross_workflow_jump_rejected():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()

    expect_runtime_error(
        ui.open_final_word_tool
    )

    expect_runtime_error(
        ui.open_validate_phrase
    )

    expect_runtime_error(
        ui.open_about_security
    )

    assert ui.state == UIState.DICE_ENTRY


def test_final_word_invalid_mode_rejected():
    ui = new_ui()

    ui.open_final_word_tool()

    expect_value_error(
        lambda: ui.choose_final_word_mode(18)
    )

    assert ui.state == UIState.FINAL_WORD_MODE
    assert ui.expected_word_input is None


def test_final_word_destroy_clears_input_state():
    ui = new_ui()

    ui.open_final_word_tool()
    ui.choose_final_word_mode(24)

    assert ui.expected_word_input == 23
    assert ui.sensitive_data_present is True

    ui.request_destroy()
    ui.destroy_session()

    assert ui.state == UIState.HOME
    assert ui.expected_word_input is None
    assert ui.sensitive_data_present is False


def test_validation_cannot_complete_without_word_count():
    ui = new_ui()

    ui.open_validate_phrase()

    expect_runtime_error(
        ui.validation_complete
    )

    assert ui.state == UIState.VALIDATE_PHRASE_ENTRY


def test_invalid_validation_count_rejected():
    ui = new_ui()

    ui.open_validate_phrase()

    expect_value_error(
        lambda: ui.choose_validation_word_count(15)
    )

    assert ui.expected_word_input is None


def test_validation_exit_clears_sensitive_state():
    ui = new_ui()

    ui.open_validate_phrase()
    ui.choose_validation_word_count(24)
    ui.validation_complete()

    assert ui.state == UIState.VALIDATE_PHRASE_RESULT
    assert ui.sensitive_data_present is True

    ui.leave_sensitive_workflow_to_home()

    assert ui.state == UIState.HOME
    assert ui.expected_word_input is None
    assert ui.sensitive_data_present is False


def test_sanity_restart_clears_old_sensitive_state():
    ui = new_ui()

    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()
    ui.dice_entry_to_review()
    ui.shake_confirm_complete()
    ui.dice_complete_continue(warnings=True)

    assert ui.sensitive_data_present is True

    ui.sanity_restart()

    assert ui.state == UIState.DICE_INTRO
    assert ui.word_count == 24
    assert ui.dice_session_active is True
    assert ui.mnemonic_generated is False
    assert ui.sensitive_data_present is False


def test_boot_cannot_run_twice():
    ui = new_ui()

    expect_runtime_error(
        ui.boot_complete
    )

    assert ui.state == UIState.HOME


def main():
    tests = [
        test_generate_from_home_rejected,
        test_begin_dice_without_session_rejected,
        test_invalid_dice_word_count_rejected,
        test_generate_too_early_rejected,
        test_destroy_from_home_rejected,
        test_double_destroy_request_rejected,
        test_destroy_go_back_preserves_session,
        test_destroy_clears_dice_workflow,
        test_destroy_after_mnemonic_clears_state,
        test_old_workflow_cannot_continue_after_destroy,
        test_cross_workflow_jump_rejected,
        test_final_word_invalid_mode_rejected,
        test_final_word_destroy_clears_input_state,
        test_validation_cannot_complete_without_word_count,
        test_invalid_validation_count_rejected,
        test_validation_exit_clears_sensitive_state,
        test_sanity_restart_clears_old_sensitive_state,
        test_boot_cannot_run_twice,
    ]

    for test in tests:
        test()

    print(
        f"PASS: {len(tests)} UI abuse and "
        "security-navigation tests passed"
    )


if __name__ == "__main__":
    main()
