#!/usr/bin/env python3
"""Verify the original AKAO catalog against a supplied USA executable."""
import argparse
import json
import pathlib
import re
import struct


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=pathlib.Path)
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]
    catalog = json.loads((root / "docs/AKAO_OPCODE_TABLE.json").read_text())
    data = args.executable.read_bytes()
    if data[:8] != b"PS-X EXE":
        raise ValueError("Expected a PS-X EXE")
    load, size = struct.unpack_from("<II", data, 0x18)
    if size > len(data) - 0x800:
        raise ValueError("Truncated executable")

    def offset(address, length):
        relative = address - load
        if relative < 0 or relative + length > size:
            raise ValueError("Catalog address outside executable text/data")
        return relative + 0x800

    handlers = set()
    entries = 0
    for page in catalog["pages"]:
        extended = page["page"] == "extended"
        expected_opcodes = list(range(32)) if extended else list(range(0xA0, 256))
        if [e["opcode"] for e in page["entries"]] != expected_opcodes:
            raise ValueError("Missing or reordered opcode entries")
        length_address = 0x8009B8BC if extended else 0x8009B85C
        for index, entry in enumerate(page["entries"]):
            address = int(page["table_address"], 16) + entry["opcode"] * 4
            actual = struct.unpack_from("<I", data, offset(address, 4))[0]
            if actual != int(entry["address"], 16):
                raise ValueError(f"Handler address mismatch: {page['page']} {entry['opcode']}")
            if data[offset(length_address + index, 1)] != entry["lookahead_skip"]:
                raise ValueError("Lookahead skip mismatch")
            if not entry["handler"] or not entry["source_files"]:
                raise ValueError("Unresolved handler")
            for source in entry["source_files"]:
                pattern = r"\b" + re.escape(entry["handler"]) + r"\s*\([^;{}]*\)\s*\{"
                if not re.search(pattern, (root / source).read_text()):
                    raise ValueError(f"Missing original procedure: {entry['handler']}")
            handlers.add(entry["handler"])
            entries += 1
    print(f"Original AKAO catalog verified: {entries} entries, {len(handlers)} source procedures")
    print("This verifies catalog addresses/skip values and source presence, not runtime dispatch or audio.")


if __name__ == "__main__":
    main()
