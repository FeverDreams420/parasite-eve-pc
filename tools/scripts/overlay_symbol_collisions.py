#!/usr/bin/env python3
"""Find global symbols that more than one overlay (or an overlay and main) defines.

The overlays share one load window, so each links on its own. A single-
executable PC port needs every global name to be unique. For each global
symbol defined in the object files of two or more built overlays
(``build/USA/overlays/*/**/*.o``) this classifies the definitions:

* ``identical``   same section kind, same size, same bytes once relocated words
                  are masked, and the same relocation targets (shared sources
                  such as ``room_lib`` compiled into each room): a harmless
                  duplicate, one definition can serve every overlay;
* ``conflicting`` anything else: different things that happen to share a name.

A symbol that an overlay defines and ``build/USA/main.elf`` also defines is a
third class (``overlay_vs_main``).

  overlay_symbol_collisions.py                 print the summary
  overlay_symbol_collisions.py --write         also write docs/OVERLAY_SYMBOL_COLLISIONS.md
  overlay_symbol_collisions.py --check         fail when a new conflicting or
                                               overlay-vs-main name appears
  overlay_symbol_collisions.py --update-baseline
  overlay_symbol_collisions.py --rename-map FILE
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build/USA"
OVERLAYS = BUILD / "overlays"
MAIN_ELF = BUILD / "main.elf"
CONFIGS = ROOT / "configs/USA/overlays"
BASELINE = ROOT / "tools/overlay_collisions_baseline.json"
REPORT = ROOT / "docs/OVERLAY_SYMBOL_COLLISIONS.md"

# Names a splat/auto-naming pass produced; renamed by other tools, so the
# ratchet and the rename work treat them separately.
AUTO = re.compile(r"^(__D_[0-9A-Fa-f]{8}_databin|(func|D|jtbl|jpt|L|LAB|lbl|sub|loc|BSS|B|S|P)_[0-9A-Fa-f]{6,8})$")

# Sources compiled into several overlays. A name they mention is the room
# interface: each overlay defines it, the shared code refers to it.
SHARED_DIRS = ("src/overlays/room_lib", "include/pe1")


def is_auto(name: str) -> bool:
    return bool(AUTO.match(name))


def shared_words() -> set[str]:
    words: set[str] = set()
    for d in SHARED_DIRS:
        for f in (ROOT / d).rglob("*"):
            if f.suffix in (".c", ".h", ".inc") and f.is_file():
                words.update(re.findall(r"\w+", f.read_text(errors="replace")))
    return words


def classify(name: str, in_main: bool, shared: set[str]) -> str:
    """auto | main | interface | private (the last is what must be renamed)."""
    if is_auto(name):
        return "auto"
    if in_main:
        return "main"
    return "interface" if name in shared else "private"


def section_kind(sec) -> str:
    name = sec.name
    if sec["sh_type"] == "SHT_NOBITS":
        return "bss"
    return name


def scan_object(path: Path, overlay: str, out: dict) -> None:
    with path.open("rb") as fh:
        elf = ELFFile(fh)
        symtab = elf.get_section_by_name(".symtab")
        if symtab is None:
            return
        syms = list(symtab.iter_symbols())
        sections = list(elf.iter_sections())
        relocs = defaultdict(list)
        for sec in sections:
            if isinstance(sec, RelocationSection):
                target = sections[sec["sh_info"]]
                for r in sec.iter_relocations():
                    s = syms[r["r_info_sym"]]
                    if s["st_info"]["type"] == "STT_SECTION":
                        sname = "<sec:%s>" % sections[s["st_shndx"]].name
                    else:
                        sname = s.name
                    relocs[target.name].append((r["r_offset"], r["r_info_type"], sname))
        # Label extents inside each section, for symbols without st_size.
        starts = defaultdict(list)
        for s in syms:
            shndx = s["st_shndx"]
            if isinstance(shndx, int) and s["st_info"]["type"] != "STT_SECTION" \
                    and s["st_info"]["type"] != "STT_FILE":
                starts[shndx].append(s["st_value"])
        for k in starts:
            starts[k].sort()
        for s in syms:
            bind = s["st_info"]["bind"]
            if bind not in ("STB_GLOBAL", "STB_WEAK") or not s.name:
                continue
            shndx = s["st_shndx"]
            if shndx == "SHN_UNDEF" or shndx == "SHN_ABS":
                continue
            if shndx == "SHN_COMMON":
                out[s.name].append((overlay, str(path), "common", s["st_size"],
                                    hashlib.sha1(b"common").hexdigest()))
                continue
            sec = sections[shndx]
            off = s["st_value"]
            size = s["st_size"]
            if size == 0:
                nxt = [v for v in starts[shndx] if v > off]
                size = (nxt[0] if nxt else sec["sh_size"]) - off
            kind = section_kind(sec)
            if kind == "bss":
                digest = "bss"
            else:
                data = bytearray(sec.data()[off:off + size])
                rel = []
                for roff, rtype, rsym in relocs.get(sec.name, ()):
                    if off <= roff < off + size:
                        rel.append((roff - off, rtype, rsym))
                        for i in range(roff - off, min(roff - off + 4, size)):
                            data[i] = 0
                rel.sort()
                digest = hashlib.sha1(bytes(data) + repr(rel).encode()).hexdigest()
            out[s.name].append((overlay, str(path), kind, size, digest))


def main_globals() -> set[str]:
    names = set()
    with MAIN_ELF.open("rb") as fh:
        symtab = ELFFile(fh).get_section_by_name(".symtab")
        for s in symtab.iter_symbols():
            if s["st_info"]["bind"] in ("STB_GLOBAL", "STB_WEAK") and s.name \
                    and s["st_shndx"] not in ("SHN_UNDEF", "SHN_ABS"):
                names.add(s.name)
    return names


def built_state() -> tuple[int, int]:
    configured = len(list(CONFIGS.glob("*.yaml")))
    built = len(list(OVERLAYS.glob("*/*.elf"))) if OVERLAYS.is_dir() else 0
    return built, configured


def collect() -> dict:
    defs: dict[str, list] = defaultdict(list)
    for ov in sorted(p for p in OVERLAYS.iterdir() if p.is_dir()):
        for obj in sorted(ov.rglob("*.o")):
            scan_object(obj, ov.name, defs)
    main = main_globals()
    identical, conflicting = {}, {}
    for name, lst in defs.items():
        ovs = sorted({d[0] for d in lst})
        if len(ovs) < 2:
            continue
        # one definition per overlay is expected; take its signature
        sigs = {}
        for ov, _p, kind, size, digest in lst:
            sigs.setdefault(ov, set()).add((kind, size, digest))
        flat = set().union(*sigs.values())
        info = {"overlays": ovs, "kinds": sorted({d[2] for d in lst}),
                "sizes": sorted({d[3] for d in lst}),
                "variants": len(flat)}
        (identical if len(flat) == 1 else conflicting)[name] = info
    overlay_main = {n: sorted({d[0] for d in defs[n]}) for n in defs if n in main}
    shared = shared_words()
    for n, i in conflicting.items():
        i["class"] = classify(n, n in main, shared)
    return {"identical": identical, "conflicting": conflicting,
            "overlay_vs_main": overlay_main, "total_names": len(defs)}


def counts(res: dict) -> dict:
    conf = res["conflicting"]
    om = res["overlay_vs_main"]
    return {
        "identical": len(res["identical"]),
        "conflicting": len(conf),
        "conflicting_named": sum(1 for n in conf if not is_auto(n)),
        "conflicting_auto": sum(1 for n in conf if is_auto(n)),
        "conflicting_interface": sum(1 for i in conf.values() if i["class"] == "interface"),
        "conflicting_main": sum(1 for i in conf.values() if i["class"] == "main"),
        "conflicting_private": sum(1 for i in conf.values() if i["class"] == "private"),
        "overlay_vs_main": len(om),
        "overlay_vs_main_named": sum(1 for n in om if not is_auto(n)),
    }


def render(res: dict) -> str:
    c = counts(res)
    conf, om = res["conflicting"], res["overlay_vs_main"]
    out = ["# Overlay symbol collisions", "",
           "Generated by `tools/scripts/overlay_symbol_collisions.py --write` from the built",
           "overlay objects (`make overlay-build-all`) and `build/USA/main.elf`. Do not edit.",
           "",
           "A single-executable port needs unique global names. A name defined by two or",
           "more overlays is `identical` when section kind, size, bytes (relocated words",
           "masked) and relocation targets agree - one shared definition serves them all -",
           "and `conflicting` otherwise. Rules for resolving the conflicting names are in",
           "`docs/CONVENTIONS.md` (Overlays).", "",
           "| Class | Names |", "| --- | ---: |",
           f"| Global names defined by any overlay | {res['total_names']} |",
           f"| Defined in more than one overlay, identical | {c['identical']} |",
           f"| Defined in more than one overlay, conflicting | {c['conflicting']} |",
           f"| - of which already non-auto names | {c['conflicting_named']} |",
           f"|   - room interface (a shared source names it) | {c['conflicting_interface']} |",
           f"|   - also a main symbol | {c['conflicting_main']} |",
           f"|   - private to one overlay's own code (must be renamed) | {c['conflicting_private']} |",
           f"| - of which auto names (`func_`/`D_`/`jtbl_`/`__D_..._databin`) | {c['conflicting_auto']} |",
           f"| Overlay global that main also defines | {c['overlay_vs_main']} |",
           f"| - of which non-auto names | {c['overlay_vs_main_named']} |", ""]

    def table(title, items, note=""):
        out.append(title)
        out.append("")
        if note:
            out.extend([note, ""])
        if not items:
            out.extend(["None.", ""])
            return
        out.extend(["| Name | Overlays | Kinds | Sizes |", "| --- | --- | --- | --- |"])
        for n, i in items:
            ovs = i["overlays"]
            shown = ", ".join(ovs[:6]) + (f", ... ({len(ovs)})" if len(ovs) > 6 else "")
            out.append(f"| `{n}` | {shown} | {','.join(i['kinds'])} | {','.join(map(str, i['sizes']))} |")
        out.append("")

    for cls, title, note in (
            ("private", "## Conflicting, private to an overlay",
             "Rename with the overlay prefix (docs/CONVENTIONS.md, Overlays)."),
            ("interface", "## Conflicting, room interface",
             "Each overlay defines the name and a source shared by several overlays "
             "(`src/overlays/room_lib`, `include/pe1`) refers to it, so the name cannot "
             "differ per overlay in the source. The single-executable link gives each "
             "overlay's definition its overlay-specific name (`--rename-map`)."),
            ("main", "## Conflicting, also a main symbol",
             "`sym.main.txt` places a main symbol inside the overlay window.")):
        table(title, sorted((n, i) for n, i in conf.items() if i["class"] == cls), note)
    auto = sorted((n, i) for n, i in conf.items() if is_auto(n))
    table("## Conflicting, auto names", auto,
          "Left to the concurrent auto-name renaming; listed here only.")
    out.append("## Overlay global also defined by main")
    out.append("")
    if om:
        out.extend(["| Name | Overlays |", "| --- | --- |"])
        for n in sorted(om):
            ovs = om[n]
            out.append(f"| `{n}` | {', '.join(ovs[:6])}{', ...' if len(ovs) > 6 else ''} |")
    else:
        out.append("None.")
    out.append("")
    return "\n".join(out)


def rename_map(res: dict) -> list[str]:
    """overlay<TAB>old<TAB>new for every conflicting name; new = old__overlay."""
    lines = []
    for n, i in sorted(res["conflicting"].items()):
        for ov in i["overlays"]:
            lines.append(f"{ov}\t{n}\t{n}__{ov}")
    return lines


def baseline_doc(res: dict) -> dict:
    return {"counts": counts(res),
            "conflicting": sorted(res["conflicting"]),
            "overlay_vs_main": sorted(res["overlay_vs_main"])}


def check(res: dict, require: bool) -> int:
    base = json.loads(BASELINE.read_text())
    bad = []
    new_conf = sorted(set(res["conflicting"]) - set(base["conflicting"]))
    new_om = sorted(set(res["overlay_vs_main"]) - set(base["overlay_vs_main"]))
    if new_conf:
        bad.append("new conflicting overlay names: " + ", ".join(new_conf[:20]))
    if new_om:
        bad.append("new overlay names that collide with main: " + ", ".join(new_om[:20]))
    priv = sorted(n for n, i in res["conflicting"].items() if i["class"] == "private")
    if priv:
        bad.append("conflicting names private to an overlay (rename them): " + ", ".join(priv[:20]))
    c, b = counts(res), base["counts"]
    for key in ("conflicting", "overlay_vs_main"):
        if c[key] > b[key]:
            bad.append(f"{key} grew {b[key]} -> {c[key]}")
    if bad:
        print("overlay-collision-check: FAIL")
        for line in bad:
            print("  " + line)
        print("Give per-overlay symbols an overlay-specific name (docs/CONVENTIONS.md),")
        print("or share one definition if the content really is identical.")
        return 1
    better = [k for k in ("conflicting", "overlay_vs_main") if c[k] < b[k]]
    if better:
        print("overlay-collision-check: improved (" + ", ".join(
            f"{k} {b[k]} -> {c[k]}" for k in better) + "); run `make overlay-collision-baseline`")
    print(f"OK: conflicting={c['conflicting']} overlay_vs_main={c['overlay_vs_main']} "
          f"identical={c['identical']}")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--update-baseline", action="store_true")
    ap.add_argument("--require-overlays", action="store_true")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--rename-map", type=Path, metavar="FILE",
                    help="write overlay/old/new lines for the single-executable link")
    args = ap.parse_args(argv)
    built, configured = built_state()
    if not MAIN_ELF.is_file() or built < configured:
        msg = f"overlay-collision-check: {built}/{configured} overlays built (or main.elf missing)"
        if args.require_overlays or args.update_baseline or args.write:
            sys.exit(msg + "; run `make overlay-build-all` first")
        print(msg + ", skipped")
        return 0
    res = collect()
    if args.json:
        print(json.dumps(counts(res), indent=1))
    if args.write:
        REPORT.write_text(render(res))
        print(f"wrote {REPORT.relative_to(ROOT)}")
    if args.update_baseline:
        BASELINE.write_text(json.dumps(baseline_doc(res), indent=1) + "\n")
        print(f"wrote {BASELINE.relative_to(ROOT)}")
    if args.rename_map:
        args.rename_map.write_text("\n".join(rename_map(res)) + "\n")
        print(f"wrote {args.rename_map}")
    if args.check:
        return check(res, args.require_overlays)
    if not (args.write or args.update_baseline or args.json):
        print(json.dumps(counts(res), indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
