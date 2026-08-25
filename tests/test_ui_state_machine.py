from core.ui_state_machine import UIState, UIStateMachine


def test_boot():
    ui = UIStateMachine()

    assert ui.state == UIState.BOOT

    ui.boot_complete()

    assert ui.state == UIState.HOME
    assert ui.sensitive_data_present is False


def test_dice_navigation():
    ui = UIStateMachine()
    ui.boot_complete()

    ui.open_create_from_dice()
    assert ui.state == UIState.DICE_WORD_COUNT

    ui.choose_dice_word_count(24)

    assert ui.state == UIState.DICE_INTRO
    assert ui.word_count == 24
    assert ui.dice_session_active is True

    ui.begin_dice_entry()

    assert ui.state == UIState.DICE_ENTRY
    assert ui.sensitive_data_present is True

    ui.dice_entry_to_review()
    assert ui.state == UIState.DICE_SHAKE_REVIEW

    ui.shake_confirm_complete()
    assert ui.state == UIState.DICE_COMPLETE

    ui.dice_complete_continue(warnings=False)
    assert ui.state == UIState.DICE_GENERATE_CONFIRM

    ui.generate_mnemonic()

    assert ui.state == UIState.MNEMONIC_WORD_VIEW
    assert ui.mnemonic_generated is True

    ui.mnemonic_words_complete()
    assert ui.state == UIState.MNEMONIC_FULL_REVIEW

    ui.mnemonic_review_finish()
    assert ui.state == UIState.SESSION_DESTROY_CONFIRM

    ui.destroy_session()

    assert ui.state == UIState.HOME
    assert ui.word_count is None
    assert ui.dice_session_active is False
    assert ui.mnemonic_generated is False
    assert ui.sensitive_data_present is False


def test_destroy_cancel_go_back():
    ui = UIStateMachine()
    ui.boot_complete()
    ui.open_create_from_dice()
    ui.choose_dice_word_count(12)
    ui.begin_dice_entry()

    ui.request_destroy()

    assert ui.state == UIState.SESSION_DESTROY_CONFIRM

    ui.destroy_go_back()

    assert ui.state == UIState.DICE_ENTRY
    assert ui.sensitive_data_present is True


def test_sanity_warning_continue():
    ui = UIStateMachine()
    ui.boot_complete()
    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()
    ui.dice_entry_to_review()
    ui.shake_confirm_complete()

    ui.dice_complete_continue(warnings=True)

    assert ui.state == UIState.DICE_SANITY_WARNING

    ui.sanity_continue_anyway()

    assert ui.state == UIState.DICE_GENERATE_CONFIRM


def test_sanity_restart():
    ui = UIStateMachine()
    ui.boot_complete()
    ui.open_create_from_dice()
    ui.choose_dice_word_count(24)
    ui.begin_dice_entry()
    ui.dice_entry_to_review()
    ui.shake_confirm_complete()
    ui.dice_complete_continue(warnings=True)

    ui.sanity_restart()

    assert ui.state == UIState.DICE_INTRO
    assert ui.word_count == 24
    assert ui.dice_session_active is True
    assert ui.mnemonic_generated is False
    assert ui.sensitive_data_present is False


def test_final_word_flow():
    ui = UIStateMachine()
    ui.boot_complete()

    ui.open_final_word_tool()
    assert ui.state == UIState.FINAL_WORD_MODE

    ui.choose_final_word_mode(24)

    assert ui.state == UIState.FINAL_WORD_ENTRY
    assert ui.expected_word_input == 23
    assert ui.sensitive_data_present is True

    ui.final_word_entry_complete()
    assert ui.state == UIState.FINAL_WORD_RESULTS

    ui.request_destroy()
    ui.destroy_session()

    assert ui.state == UIState.HOME
    assert ui.expected_word_input is None
    assert ui.sensitive_data_present is False


def test_validation_flow():
    ui = UIStateMachine()
    ui.boot_complete()

    ui.open_validate_phrase()

    assert ui.state == UIState.VALIDATE_PHRASE_ENTRY
    assert ui.sensitive_data_present is True

    ui.choose_validation_word_count(12)
    assert ui.expected_word_input == 12

    ui.validation_complete()

    assert ui.state == UIState.VALIDATE_PHRASE_RESULT

    ui.leave_sensitive_workflow_to_home()

    assert ui.state == UIState.HOME
    assert ui.expected_word_input is None
    assert ui.sensitive_data_present is False


def test_invalid_transition_rejected():
    ui = UIStateMachine()
    ui.boot_complete()

    try:
        ui.generate_mnemonic()

    except RuntimeError:
        pass

    else:
        raise AssertionError(
            "Invalid generation transition was not rejected"
        )


def test_invalid_word_counts_rejected():
    ui = UIStateMachine()
    ui.boot_complete()
    ui.open_create_from_dice()

    try:
        ui.choose_dice_word_count(18)

    except ValueError:
        pass

    else:
        raise AssertionError(
            "Invalid dice word count was not rejected"
        )


def main():
    tests = [
        test_boot,
        test_dice_navigation,
        test_destroy_cancel_go_back,
        test_sanity_warning_continue,
        test_sanity_restart,
        test_final_word_flow,
        test_validation_flow,
        test_invalid_transition_rejected,
        test_invalid_word_counts_rejected,
    ]

    for test in tests:
        test()

    print(
        "PASS: UI state machine navigation and "
        "destruction rules"
    )


if __name__ == "__main__":
    main()
