#!/usr/bin/env python3
"""Verify the original AKAO and resident script catalogs against a USA executable."""
import argparse
import json
import pathlib
import re
import struct


def verify_script_catalog(root, data, offset):
    catalog = json.loads((root / "docs/SCRIPT_OPCODE_TABLE.json").read_text())
    entries = catalog["entries"]
    if [entry["opcode"] for entry in entries] != list(range(241)):
        raise ValueError("Missing or reordered resident script entries")
    source = (root / "src/main/script/flow_opcodes.c").read_text()
    table = re.search(r"g_ScriptQueueOpcodeHandlers\[8192\][^;={]*=\s*\{([^}]+)\}", source)
    if not table:
        raise ValueError("Missing native resident script table")
    bindings = re.findall(r"\[(\d+)\]\s*=\s*(\w+)", table[1])
    native = {int(op): name for op, name in bindings}
    if len(native) != len(bindings) or any(op >= len(entries) for op in native):
        raise ValueError("Duplicate or out-of-catalog resident script binding")
    bound = 0
    for entry in entries:
        op = entry["opcode"]
        address = int(entry["original_address"], 16)
        actual = struct.unpack_from("<I", data, offset(0x800910A0 + op * 4, 4))[0]
        if actual != address:
            raise ValueError(f"Resident script address mismatch: {op}")
        if bool(address) != bool(entry["function"]):
            raise ValueError(f"Unresolved resident script procedure: {op}")
        if native.get(op) != entry["function"]:
            raise ValueError(f"Native resident script handler mismatch: {op}")
        bound += bool(address)
    return len(entries), bound


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
    dispatch = (root / "src/main/akao/sequencer.c").read_text()
    for page in catalog["pages"]:
        extended = page["page"] == "extended"
        expected_opcodes = list(range(32)) if extended else list(range(0xA0, 256))
        if [e["opcode"] for e in page["entries"]] != expected_opcodes:
            raise ValueError("Missing or reordered opcode entries")
        length_address = 0x8009B8BC if extended else 0x8009B85C
        table_name = "g_AkaoExtOpcodeHandlers" if extended else "g_AkaoOpcodeHandlers"
        table = re.search(table_name + r"\[\d+\]\s*=\s*\{([^}]+)\}", dispatch)
        if not table:
            raise ValueError("Missing native dispatch table")
        native = {int(op, 16): name for op, name in re.findall(
            r"\[(0x[0-9A-Fa-f]+)\]\s*=\s*(\w+)", table[1])}
        if len(native) != len(expected_opcodes):
            raise ValueError("Native dispatch entry count mismatch")
        for index, entry in enumerate(page["entries"]):
            address = int(page["table_address"], 16) + entry["opcode"] * 4
            actual = struct.unpack_from("<I", data, offset(address, 4))[0]
            if actual != int(entry["address"], 16):
                raise ValueError(f"Handler address mismatch: {page['page']} {entry['opcode']}")
            if data[offset(length_address + index, 1)] != entry["lookahead_skip"]:
                raise ValueError("Lookahead skip mismatch")
            if not entry["handler"] or not entry["source_files"]:
                raise ValueError("Unresolved handler")
            if native.get(entry["opcode"]) != entry["handler"]:
                raise ValueError("Native dispatch handler mismatch")
            for source in entry["source_files"]:
                pattern = r"\b" + re.escape(entry["handler"]) + r"\s*\([^;{}]*\)\s*\{"
                if not re.search(pattern, (root / source).read_text()):
                    raise ValueError(f"Missing original procedure: {entry['handler']}")
            handlers.add(entry["handler"])
            entries += 1
    script_entries, script_bound = verify_script_catalog(root, data, offset)
    print(f"Original AKAO catalog verified: {entries} entries, {len(handlers)} source procedures")
    print(f"Resident script catalog verified: {script_entries} entries, "
          f"{script_bound} native bindings, {script_entries - script_bound} original null entries")
    print("This verifies catalog addresses/skip values and source presence, not runtime dispatch or audio.")


if __name__ == "__main__":
    main()
