from enum import Enum, auto


class UIState(Enum):
    BOOT = auto()
    HOME = auto()

    DICE_WORD_COUNT = auto()
    DICE_INTRO = auto()
    DICE_ENTRY = auto()
    DICE_SHAKE_REVIEW = auto()
    DICE_COMPLETE = auto()
    DICE_SANITY_WARNING = auto()
    DICE_GENERATE_CONFIRM = auto()
    MNEMONIC_WORD_VIEW = auto()
    MNEMONIC_FULL_REVIEW = auto()
    SESSION_DESTROY_CONFIRM = auto()

    FINAL_WORD_MODE = auto()
    FINAL_WORD_ENTRY = auto()
    FINAL_WORD_RESULTS = auto()

    VALIDATE_PHRASE_ENTRY = auto()
    VALIDATE_PHRASE_RESULT = auto()

    ABOUT_SECURITY = auto()


SENSITIVE_STATES = {
    UIState.DICE_ENTRY,
    UIState.DICE_SHAKE_REVIEW,
    UIState.DICE_COMPLETE,
    UIState.DICE_SANITY_WARNING,
    UIState.DICE_GENERATE_CONFIRM,
    UIState.MNEMONIC_WORD_VIEW,
    UIState.MNEMONIC_FULL_REVIEW,
    UIState.SESSION_DESTROY_CONFIRM,
    UIState.FINAL_WORD_ENTRY,
    UIState.FINAL_WORD_RESULTS,
    UIState.VALIDATE_PHRASE_ENTRY,
    UIState.VALIDATE_PHRASE_RESULT,
}


class UIStateMachine:
    """
    Hardware-independent V1 UI navigation model.

    This class does not perform cryptography and does not prove secure
    memory erasure. It exists to make navigation and destruction rules
    deterministic and testable before the LVGL implementation.
    """

    def __init__(self):
        self.state = UIState.BOOT

        self.word_count = None
        self.expected_word_input = None

        self.dice_session_active = False
        self.mnemonic_generated = False
        self.sensitive_data_present = False

        self._destroy_return_state = None

    @property
    def is_sensitive_state(self):
        return self.state in SENSITIVE_STATES

    def _require(self, *allowed_states):
        if self.state not in allowed_states:
            allowed = ", ".join(
                state.name for state in allowed_states
            )

            raise RuntimeError(
                f"{self.state.name} cannot perform this action. "
                f"Allowed state(s): {allowed}"
            )

    def _set_state(self, state):
        self.state = state

    def _mark_sensitive(self):
        self.sensitive_data_present = True

    def _clear_sensitive_session(self):
        self.word_count = None
        self.expected_word_input = None

        self.dice_session_active = False
        self.mnemonic_generated = False
        self.sensitive_data_present = False

        self._destroy_return_state = None

    # ------------------------------------------------------------
    # BOOT / HOME
    # ------------------------------------------------------------

    def boot_complete(self):
        self._require(UIState.BOOT)

        self._clear_sensitive_session()
        self._set_state(UIState.HOME)

    def open_create_from_dice(self):
        self._require(UIState.HOME)
        self._set_state(UIState.DICE_WORD_COUNT)

    def open_final_word_tool(self):
        self._require(UIState.HOME)
        self._set_state(UIState.FINAL_WORD_MODE)

    def open_validate_phrase(self):
        self._require(UIState.HOME)

        self.expected_word_input = None
        self._mark_sensitive()

        self._set_state(UIState.VALIDATE_PHRASE_ENTRY)

    def open_about_security(self):
        self._require(UIState.HOME)
        self._set_state(UIState.ABOUT_SECURITY)

    def about_back(self):
        self._require(UIState.ABOUT_SECURITY)
        self._set_state(UIState.HOME)

    # ------------------------------------------------------------
    # DICE WORKFLOW
    # ------------------------------------------------------------

    def choose_dice_word_count(self, word_count):
        self._require(UIState.DICE_WORD_COUNT)

        if word_count not in (12, 24):
            raise ValueError(
                "word_count must be 12 or 24"
            )

        self.word_count = word_count
        self.dice_session_active = True
        self.mnemonic_generated = False
        self.sensitive_data_present = False

        self._set_state(UIState.DICE_INTRO)

    def dice_word_count_back(self):
        self._require(UIState.DICE_WORD_COUNT)
        self._set_state(UIState.HOME)

    def begin_dice_entry(self):
        self._require(UIState.DICE_INTRO)

        if not self.dice_session_active:
            raise RuntimeError(
                "No active dice session"
            )

        self._mark_sensitive()
        self._set_state(UIState.DICE_ENTRY)

    def dice_entry_to_review(self):
        self._require(UIState.DICE_ENTRY)
        self._set_state(UIState.DICE_SHAKE_REVIEW)

    def shake_review_reenter(self):
        self._require(UIState.DICE_SHAKE_REVIEW)
        self._set_state(UIState.DICE_ENTRY)

    def shake_confirm_more_required(self):
        self._require(UIState.DICE_SHAKE_REVIEW)
        self._set_state(UIState.DICE_ENTRY)

    def shake_confirm_complete(self):
        self._require(UIState.DICE_SHAKE_REVIEW)
        self._set_state(UIState.DICE_COMPLETE)

    def dice_complete_continue(self, warnings=False):
        self._require(UIState.DICE_COMPLETE)

        if warnings:
            self._set_state(
                UIState.DICE_SANITY_WARNING
            )

        else:
            self._set_state(
                UIState.DICE_GENERATE_CONFIRM
            )

    def sanity_continue_anyway(self):
        self._require(
            UIState.DICE_SANITY_WARNING
        )

        self._set_state(
            UIState.DICE_GENERATE_CONFIRM
        )

    def sanity_restart(self):
        self._require(
            UIState.DICE_SANITY_WARNING
        )

        previous_word_count = self.word_count

        self._clear_sensitive_session()

        self.word_count = previous_word_count
        self.dice_session_active = True

        self._set_state(UIState.DICE_INTRO)

    def generate_back(self):
        self._require(
            UIState.DICE_GENERATE_CONFIRM
        )

        self._set_state(UIState.DICE_COMPLETE)

    def generate_mnemonic(self):
        self._require(
            UIState.DICE_GENERATE_CONFIRM
        )

        if not self.dice_session_active:
            raise RuntimeError(
                "No active dice session"
            )

        self.mnemonic_generated = True
        self._mark_sensitive()

        self._set_state(
            UIState.MNEMONIC_WORD_VIEW
        )

    def mnemonic_words_complete(self):
        self._require(
            UIState.MNEMONIC_WORD_VIEW
        )

        if not self.mnemonic_generated:
            raise RuntimeError(
                "Mnemonic has not been generated"
            )

        self._set_state(
            UIState.MNEMONIC_FULL_REVIEW
        )

    def mnemonic_review_finish(self):
        self._require(
            UIState.MNEMONIC_FULL_REVIEW
        )

        self.request_destroy()

    # ------------------------------------------------------------
    # FINAL WORD TOOL
    # ------------------------------------------------------------

    def choose_final_word_mode(self, total_words):
        self._require(UIState.FINAL_WORD_MODE)

        if total_words == 12:
            self.expected_word_input = 11

        elif total_words == 24:
            self.expected_word_input = 23

        else:
            raise ValueError(
                "total_words must be 12 or 24"
            )

        self._mark_sensitive()
        self._set_state(UIState.FINAL_WORD_ENTRY)

    def final_word_mode_back(self):
        self._require(UIState.FINAL_WORD_MODE)
        self._set_state(UIState.HOME)

    def final_word_entry_complete(self):
        self._require(UIState.FINAL_WORD_ENTRY)
        self._set_state(UIState.FINAL_WORD_RESULTS)

    def final_word_results_back(self):
        self._require(UIState.FINAL_WORD_RESULTS)

        self._set_state(UIState.FINAL_WORD_ENTRY)

    def final_word_results_home(self):
        self._require(UIState.FINAL_WORD_RESULTS)

        self._clear_sensitive_session()
        self._set_state(UIState.HOME)

    # ------------------------------------------------------------
    # VALIDATION TOOL
    # ------------------------------------------------------------

    def choose_validation_word_count(
        self,
        word_count,
    ):
        self._require(
            UIState.VALIDATE_PHRASE_ENTRY
        )

        if word_count not in (12, 24):
            raise ValueError(
                "word_count must be 12 or 24"
            )

        self.expected_word_input = word_count
        self._mark_sensitive()

    def validation_complete(self):
        self._require(
            UIState.VALIDATE_PHRASE_ENTRY
        )

        if self.expected_word_input not in (
            12,
            24,
        ):
            raise RuntimeError(
                "Validation word count has "
                "not been selected"
            )

        self._set_state(
            UIState.VALIDATE_PHRASE_RESULT
        )

    def validation_review_words(self):
        self._require(
            UIState.VALIDATE_PHRASE_RESULT
        )

        self._set_state(
            UIState.VALIDATE_PHRASE_ENTRY
        )

    def validation_start_over(self):
        self._require(
            UIState.VALIDATE_PHRASE_RESULT
        )

        self.expected_word_input = None
        self.sensitive_data_present = False

        self._set_state(
            UIState.VALIDATE_PHRASE_ENTRY
        )

    def validation_result_home(self):
        self._require(
            UIState.VALIDATE_PHRASE_RESULT
        )

        self._clear_sensitive_session()
        self._set_state(UIState.HOME)

    # ------------------------------------------------------------
    # DESTRUCTION / LEAVING SENSITIVE WORKFLOWS
    # ------------------------------------------------------------

    def request_destroy(self):
        if self.state not in SENSITIVE_STATES:
            raise RuntimeError(
                "Destroy confirmation may only "
                "be opened from a sensitive state"
            )

        if (
            self.state
            == UIState.SESSION_DESTROY_CONFIRM
        ):
            raise RuntimeError(
                "Already in destroy confirmation"
            )

        self._destroy_return_state = self.state

        self._set_state(
            UIState.SESSION_DESTROY_CONFIRM
        )

    def destroy_go_back(self):
        self._require(
            UIState.SESSION_DESTROY_CONFIRM
        )

        if self._destroy_return_state is None:
            raise RuntimeError(
                "No previous sensitive state"
            )

        previous_state = (
            self._destroy_return_state
        )

        self._destroy_return_state = None

        self._set_state(previous_state)

    def destroy_session(self):
        self._require(
            UIState.SESSION_DESTROY_CONFIRM
        )

        self._clear_sensitive_session()
        self._set_state(UIState.HOME)

    def leave_sensitive_workflow_to_home(self):
        if self.state not in SENSITIVE_STATES:
            raise RuntimeError(
                "Current state is not a "
                "sensitive workflow"
            )

        self._clear_sensitive_session()
        self._set_state(UIState.HOME)
