from core.dice_session import DiceSession
from core.seed_engine import create_seed_from_dice


class SeedCeremony:
    def __init__(self, word_count):
        self.session = DiceSession(word_count)
        self.word_count = word_count
        self._generated = False
        self._result = None

    @property
    def shake_count(self):
        return self.session.shake_count

    @property
    def total_shakes(self):
        return self.session.total_shakes

    @property
    def remaining_shakes(self):
        return self.session.remaining_shakes

    @property
    def complete(self):
        return self.session.complete

    @property
    def generated(self):
        return self._generated

    def add_shake(self, results):
        if self._generated:
            raise ValueError(
                "Seed has already been generated. Destroy or restart the session."
            )

        self.session.add_shake(results)

    def undo_last_shake(self):
        if self._generated:
            raise ValueError(
                "Cannot modify dice after seed generation."
            )

        return self.session.undo_last_shake()

    def generate(self):
        if self._generated:
            raise ValueError("Seed has already been generated.")

        if not self.complete:
            raise ValueError(
                f"Dice entry is incomplete. "
                f"{self.remaining_shakes} shakes remain."
            )

        dice = self.session.canonical_dice_string()

        self._result = create_seed_from_dice(
            dice,
            self.word_count,
        )

        self._generated = True

        return self._result

    def destroy_session(self):
        self.session.reset()
        self._result = None
        self._generated = False
