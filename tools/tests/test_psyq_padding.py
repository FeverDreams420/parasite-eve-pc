"""Padding must be an unlabeled SDK object tail, not hidden unmatched code."""
import json
import pathlib
import struct
import unittest

import yaml

ROOT = pathlib.Path(__file__).resolve().parents[2]
PROVENANCE = json.loads((ROOT / "configs/USA/psyq_provenance.json").read_text())

# SDK padding that a unit once emitted itself instead of a `pad` row: merging
# Spu_ReadFromSpu/Spu_UploadToSpu (642bcc1bd) moved the S_R.OBJ tail word into
# the unit, and the later split into psyq/libspu/s_r + s_w (e5da83a14) kept it
# in .text until the [0x76650, pad] row was restored.
ABSORBED = {0x80085E50}


def pad_manifest():
    config = yaml.safe_load((ROOT / "configs/USA/main.yaml").read_text())
    manifest = {}
    for segment in config["segments"]:
        if not isinstance(segment, dict):
            continue
        rows = segment.get("subsegments", [])
        for row, following in zip(rows, rows[1:]):
            if isinstance(row, list) and row[1] == "pad":
                end = following["start"] if isinstance(following, dict) else following[0]
                manifest[row[0] + 0x8000F800] = end - row[0]
    return manifest


class PsyqPaddingTests(unittest.TestCase):
    def test_absorbed_padding_is_restored_as_pad_rows(self):
        manifest = pad_manifest()
        approved = {int(p["address"], 16): p["size"] for p in PROVENANCE["padding"]}
        self.assertEqual({start: manifest.get(start) for start in ABSORBED},
                         {start: approved[start] for start in ABSORBED})

    def test_padding_manifest_has_sdk_boundary_evidence(self):
        manifest = pad_manifest()
        approved = {int(p["address"], 16): p["size"] for p in PROVENANCE["padding"]}
        self.assertEqual({start: manifest.get(start) for start in approved
                          if start not in ABSORBED},
                         {start: size for start, size in approved.items()
                          if start not in ABSORBED})
        for start, size in approved.items():
            with self.subTest(address=hex(start)):
                self.assertIn(size, (4, 8, 12))
                owners = [e for e in PROVENANCE["evidence"]
                          if e["scope"] == "object"
                          and int(e["address"], 16) < start
                          and int(e["address"], 16) + e["size"] == start + size]
                self.assertTrue(owners)
                for owner in owners:
                    self.assertFalse(any(start <= int(owner["address"], 16) + label["offset"]
                                         < start + size for label in owner["labels"]))

    @unittest.skipUnless((ROOT / "assets/USA/main.exe").exists(), "retail image unavailable")
    def test_retail_padding_is_zero_and_follows_return_delay_slot(self):
        retail = (ROOT / "assets/USA/main.exe").read_bytes()
        for padding in PROVENANCE["padding"]:
            offset = int(padding["address"], 16) - 0x8000F800
            with self.subTest(address=padding["address"]):
                self.assertEqual(retail[offset:offset + padding["size"]],
                                 bytes(padding["size"]))
                self.assertEqual(struct.unpack_from("<I", retail, offset - 8)[0],
                                 0x03E00008)  # jr $ra, followed by its delay slot
