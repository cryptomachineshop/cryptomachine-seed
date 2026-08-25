from core.dice_policy import required_shakes


class DiceSession:
    DICE_PER_SHAKE = 5

    def __init__(self, word_count):
        if word_count not in (12, 24):
            raise ValueError("word_count must be 12 or 24.")

        self.word_count = word_count
        self.total_shakes = required_shakes(word_count)
        self._shakes = []

    @property
    def shake_count(self):
        return len(self._shakes)

    @property
    def remaining_shakes(self):
        return self.total_shakes - self.shake_count

    @property
    def complete(self):
        return self.shake_count == self.total_shakes

    def add_shake(self, results):
        if self.complete:
            raise ValueError("Dice session is already complete.")

        if len(results) != self.DICE_PER_SHAKE:
            raise ValueError("Each shake must contain exactly five dice results.")

        normalized = []

        for result in results:
            value = str(result)

            if value not in "123456":
                raise ValueError("Dice results must be values 1 through 6.")

            normalized.append(value)

        self._shakes.append(tuple(normalized))

    def undo_last_shake(self):
        if not self._shakes:
            raise ValueError("There is no shake to undo.")

        return self._shakes.pop()

    def get_shakes(self):
        return list(self._shakes)

    def canonical_dice_string(self):
        return "".join(
            "".join(shake)
            for shake in self._shakes
        )

    def reset(self):
        self._shakes.clear()
