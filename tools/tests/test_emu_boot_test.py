import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools.scripts import emu_boot_test as ebt


def checkpoint(name: str, token: str, frame: int = 100, presented: int = 101) -> dict:
    return {"name": name, "frame": frame, "presented": presented, "screenshot": f"{name}.png",
            "size": [2, 2], "probes": {"g_SceneDispatchToken": token}, "pc": "0x80010000", "function": "f"}


class PngTests(unittest.TestCase):
    def test_a_written_png_reads_back(self):
        rgba = bytes(range(16))
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "a.png"
            ebt.write_png(path, 2, 2, rgba)
            self.assertEqual(ebt.read_png(path), (2, 2, rgba))

    def test_the_difference_counts_pixels_and_ignores_alpha(self):
        with tempfile.TemporaryDirectory() as tmp:
            a, b = Path(tmp) / "a.png", Path(tmp) / "b.png"
            ebt.write_png(a, 2, 2, bytes(16))
            ebt.write_png(b, 2, 2, bytes([0, 0, 0, 9]) + bytes([1, 0, 0, 0]) + bytes(8))
            self.assertEqual(ebt.pixel_difference(a, b), 0.25)
            ebt.write_png(b, 1, 2, bytes(8))
            self.assertIsNone(ebt.pixel_difference(a, b))


class CompareTests(unittest.TestCase):
    def run_compare(self, base: list[dict], new: list[dict]) -> list[str]:
        with tempfile.TemporaryDirectory() as tmp:
            base_dir, new_dir = Path(tmp) / "base", Path(tmp) / "new"
            base_dir.mkdir()
            new_dir.mkdir()
            for c in base:
                ebt.write_png(base_dir / c["screenshot"], 2, 2, bytes(16))
            for c in new:
                ebt.write_png(new_dir / c["screenshot"], 2, 2, bytes(16))
            (base_dir / "report.json").write_text(json.dumps({"checkpoints": base}))
            with contextlib.redirect_stdout(io.StringIO()):
                return ebt.compare({"checkpoints": new}, base_dir / "report.json", new_dir)

    def test_equal_runs_have_no_problem(self):
        runs = [checkpoint("title", "480040a9"), checkpoint("room", "481000a8")]
        self.assertEqual(self.run_compare(runs, [dict(c) for c in runs]), [])

    def test_a_token_a_frozen_picture_and_a_missing_checkpoint_are_problems(self):
        base = [checkpoint("title", "480040a9"), checkpoint("room", "481000a8"), checkpoint("end", "480200a8")]
        new = [checkpoint("title", "00000000"), checkpoint("room", "481000a8", frame=900, presented=624)]
        self.assertEqual(self.run_compare(base, new), [
            "title: scene token 00000000, base 480040a9",
            "room: the last frame presented is 624, emulated 900 (no picture since)",
            "checkpoints not reached: end"])


if __name__ == "__main__":
    unittest.main()
