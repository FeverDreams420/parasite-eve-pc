#!/usr/bin/env python3
"""Merge adjacent C units of one overlay into one translation unit.

``merge_overlay_units.py OVERLAY NEW FIRST [LAST]`` merges the `c`
subsegments FIRST..LAST of ``configs/USA/overlays/OVERLAY.yaml`` (manifest
order, which is address order) into ``src/overlays/<dir>/NEW.c``:

* the run must be contiguous in the manifest: no shared ``../room_lib`` unit,
  data or rodata subsegment lies between its members, and the members' own
  ``.rodata`` entries must be adjacent too (they collapse into one entry at
  the first offset);
* every member must carry the same per-file markers (CC1_FLAGS,
  MASPSX_FLAGS, GCC_VERSION, ...), except that a leftover ``--expand-div``
  may differ: it is cc.sh's default (every default overlay unit compiles
  byte-identically with it), and the build check confirms each merge;
* GNU-as / GAME_ASM / PSYQ_ASM units are refused.

The new source keeps the function bodies in manifest order. ``#include``
lines of headers are hoisted to the top and de-duplicated, type definitions
move up to sit between the includes and the externs (byte-identical repeats
are kept once), the units' private ``*_externs.h`` headers are folded into
one ``NEW_externs.h``, and every macro a unit defines (itself or through an
included ``.inc`` template) is ``#undef``-ed after that unit, so template
instantiations cannot leak into the next unit. Path-qualified mentions of
the old files in docs and tests are pointed at the new file.

``--dry-run`` prints the plan and writes the merged source to stdout.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
CFG = ROOT / "configs/USA/overlays"
SUBSEG = re.compile(r"^\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*([.\w]+)\s*,\s*([^\],\s]+)\s*\]")
MARKER = re.compile(r"/\*\s*(CC1_FLAGS|MASPSX_FLAGS|GCC_VERSION|ASSEMBLER|GAS_VERSION|"
                    r"MASPSX_FORCE_G0)\s*:\s*(.*?)\s*\*/")
INCLUDE = re.compile(r'^\s*#\s*include\s+([<"])([^>"]+)[>"]')
DEFINE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)", re.M)
ALIAS = re.compile(r"^typedef\s+((?:struct\s+|union\s+)?\w+)\s+(\w+)\s*;[ \t]*\n", re.M)
TYPE_START = re.compile(r"^(typedef\b|struct\s+\w+\s*\{|union\s+\w+\s*\{|enum\s+\w+\s*\{)")
EXCLUDED = (
    re.compile(r"^docs/REORGANIZATION[^/]*\.md$"),
    re.compile(r"^docs/reorganization_mapping\.csv$"),
    re.compile(r"^tools/reorg/"),
)


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, text=True,
                          capture_output=True).stdout


def markers_of(text: str) -> dict[str, str]:
    found: dict[str, str] = {}
    for key, value in MARKER.findall(text):
        found[key] = (found.get(key, "") + " " + value).strip()
    return found


def marker_key(markers: dict[str, str]) -> tuple:
    out = []
    for key, value in sorted(markers.items()):
        flags = sorted({f for f in value.split() if f != "--expand-div"})
        if flags:
            out.append((key, tuple(flags)))
    return tuple(out)


def strip_comments(text: str) -> str:
    return re.sub(r"//[^\n]*", "", re.sub(r"/\*.*?\*/", "", text, flags=re.S))


def norm_ws(text: str) -> str:
    return re.sub(r"\s+", " ", strip_comments(text)).strip()


def _is_decl(chunk: str) -> bool:
    """A brace that closes a type body or an initializer, not a function."""
    head = strip_comments(chunk).split("{", 1)[0].rstrip()
    return not head.endswith(")")


def top_level_chunks(text: str) -> list[str]:
    """Split C text into top-level chunks (preprocessor lines, declarations,
    function definitions), keeping comments and blank lines attached."""
    chunks: list[str] = []
    cur: list[str] = []
    depth = 0
    i = 0
    n = len(text)
    line_start = True
    while i < n:
        c = text[i]
        if line_start and depth == 0 and text[i:].lstrip(" \t").startswith("#"):
            j = i
            while True:
                k = text.find("\n", j)
                if k == -1:
                    k = n
                    break
                if text[k - 1] == "\\":
                    j = k + 1
                    continue
                break
            if "".join(cur).strip():
                chunks.append("".join(cur))
                cur = []
            cur.append(text[i:k + 1])
            chunks.append("".join(cur))
            cur = []
            i = k + 1
            line_start = True
            continue
        if text.startswith("/*", i):
            k = text.find("*/", i + 2)
            k = n if k == -1 else k + 2
            cur.append(text[i:k])
            i = k
            line_start = False
            continue
        if text.startswith("//", i):
            k = text.find("\n", i)
            k = n if k == -1 else k
            cur.append(text[i:k])
            i = k
            continue
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            cur.append(text[i:j + 1])
            i = j + 1
            line_start = False
            continue
        cur.append(c)
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        if depth == 0 and c in ";}":
            if c == "}" and _is_decl("".join(cur)):
                i += 1
                line_start = False
                continue
            k = text.find("\n", i)
            if k != -1 and not text[i + 1:k].strip():
                cur.append(text[i + 1:k + 1])
                i = k + 1
                line_start = True
            else:
                i += 1
                line_start = False
            chunks.append("".join(cur))
            cur = []
            continue
        line_start = c == "\n"
        i += 1
    if cur:
        chunks.append("".join(cur))
    return chunks


def defined_type_names(chunk: str) -> list[str]:
    body = strip_comments(chunk).strip()
    names = re.findall(r"^(?:typedef\s+)?(?:struct|union|enum)\s+(\w+)\s*\{", body)
    if body.startswith("typedef"):
        tail = re.sub(r"\{.*\}", "{}", body, flags=re.S)
        m = re.search(r"(\w+)\s*(?:\[[^\]]*\])*\s*;\s*$", tail)
        if m:
            names.append(m.group(1))
    return names


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("overlay")
    parser.add_argument("new")
    parser.add_argument("first")
    parser.add_argument("last", nargs="?")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    cfg = CFG / f"{args.overlay}.yaml"
    lines = cfg.read_text().splitlines(keepends=True)
    src_path = re.search(r"^\s*src_path:\s*(\S+)", "".join(lines), re.M).group(1)
    src_dir = ROOT / src_path
    last = args.last or args.first

    # ---- locate the run in the manifest
    first_idx = last_idx = None
    for idx, line in enumerate(lines):
        m = SUBSEG.match(line)
        if m and m.group(2) == "c" and m.group(3) == args.first and first_idx is None:
            first_idx = idx
        if m and m.group(2) == "c" and m.group(3) == last and first_idx is not None:
            last_idx = idx
            break
    if first_idx is None or last_idx is None:
        sys.exit(f"run {args.first}..{last} not found in {cfg.name}")
    units: list[str] = []
    offset = None
    for line in lines[first_idx:last_idx + 1]:
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        m = SUBSEG.match(line)
        if not m or m.group(2) != "c" or m.group(3).startswith("../"):
            sys.exit(f"foreign subsegment inside the run: {line.strip()}")
        units.append(m.group(3))
        offset = offset or m.group(1)
    if len(units) < 2:
        sys.exit("a run needs at least two units")
    new_unit = args.new
    if (src_dir / f"{new_unit}.c").exists() and new_unit not in units:
        sys.exit(f"{new_unit}.c exists")

    # ---- rodata entries of the members
    ro_idx = [i for i, line in enumerate(lines)
              if (m := SUBSEG.match(line)) and m.group(2) == ".rodata" and m.group(3) in units]
    if ro_idx:
        for i in range(ro_idx[0], ro_idx[-1] + 1):
            line = lines[i]
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            m = SUBSEG.match(line)
            if not m or m.group(3) not in units or m.group(2) != ".rodata":
                sys.exit(f"foreign entry between member rodata: {line.strip()}")
        ro_names = [SUBSEG.match(lines[i]).group(3) for i in ro_idx]
        if ro_names != [u for u in units if u in ro_names]:
            sys.exit("member rodata is not in text order")

    # ---- markers
    texts = {}
    for unit in units:
        text = (src_dir / f"{unit}.c").read_text()
        if re.search(r"\b(GAME_ASM_FUNCTION|PSYQ_ASM_\w+|INCLUDE_ASM)\b", text) or \
                "ASSEMBLER: GNU" in text:
            sys.exit(f"{unit}: assembler unit")
        texts[unit] = text
    keys = {u: marker_key(markers_of(t)) for u, t in texts.items()}
    if len(set(keys.values())) != 1:
        for u, k in keys.items():
            print(f"  {u}: {k}", file=sys.stderr)
        sys.exit("members have different markers")
    merged_markers: dict[str, list[str]] = {}
    for t in texts.values():
        for key, value in markers_of(t).items():
            for flag in value.split():
                merged_markers.setdefault(key, [])
                if flag not in merged_markers[key]:
                    merged_markers[key].append(flag)

    # ---- build the merged source
    hoisted: list[str] = []
    externs: list[str] = []
    externs_files: list[pathlib.Path] = []
    seen_types: set[str] = set()
    types: list[str] = []
    type_owner: dict[str, tuple[str, str]] = {}
    conflicts: list[str] = []
    bodies: list[str] = []
    for unit in units:
        text = MARKER.sub("", texts[unit])
        text = re.sub(r"^[ \t]*\n", "\n", text, flags=re.M)
        # A local alias (`typedef RoomX Particle;`) whose name an earlier
        # member already uses for another type is spelled out in this unit.
        for alias in ALIAS.finditer(text):
            target, name = alias.group(1), alias.group(2)
            key = norm_ws(alias.group(0))
            if name in type_owner and type_owner[name][1] != key:
                text = text.replace(alias.group(0), "")
                text = re.sub(r"\b" + name + r"\b", target, text)
                print(f"  inlined alias {name} = {target} in {unit}")
        # A local type body whose name an earlier member uses for another
        # layout takes the unit's role as a prefix (`PulsingPalettePairState`).
        role = re.sub(r"_[0-9A-F]{8}$", "", unit)
        role = re.sub(r"^(RoomEffect|RoomM\d+|SceneE\d+|Scene|Room|RoomLib|RoomFx|FxCommon|Memcard|Boot)_?", "", role)
        for chunk in top_level_chunks(text):
            if not TYPE_START.match(strip_comments(chunk).strip()) or ALIAS.match(chunk.strip() + "\n"):
                continue
            key = norm_ws(chunk)
            for name in defined_type_names(chunk):
                if name in type_owner and type_owner[name][1] != key:
                    if role.startswith("func_"):
                        continue
                    new_name = role + name if not name.startswith(role) else name + "B"
                    text = re.sub(r"\b" + name + r"\b", new_name, text)
                    print(f"  renamed type {name} -> {new_name} in {unit}")
        out: list[str] = []
        defined_before = False
        macros: list[str] = []
        for chunk in top_level_chunks(text):
            stripped = chunk.strip()
            inc = INCLUDE.match(stripped)
            if stripped.startswith("#") and re.match(r"#\s*define\b", stripped):
                defined_before = True
                macros += DEFINE.findall(stripped)
            if inc and inc.group(2).endswith(".h") and not defined_before:
                target = inc.group(2)
                if re.search(r"_externs(_\d+)?\.h$", target) and inc.group(1) == '"' and \
                        (src_dir / target).is_file():
                    path = src_dir / target
                    if path not in externs_files:
                        externs_files.append(path)
                        for eline in path.read_text().splitlines():
                            if re.match(r"\s*#\s*(ifndef|define|endif)\s+PE1_EXT_", eline) or \
                                    re.match(r"\s*#\s*endif\s*/\*\s*PE1_EXT_", eline):
                                continue
                            if eline.strip() and eline not in externs:
                                externs.append(eline)
                    continue
                line = stripped + "\n"
                if line not in hoisted:
                    hoisted.append(line)
                continue
            if inc and inc.group(2).endswith((".inc", ".c")):
                path = src_dir / inc.group(2)
                if path.is_file():
                    inc_text = path.read_text()
                    macros += DEFINE.findall(inc_text)
                    # A template's own externs header is guarded but spelled
                    # in the template's macros: reset its guard too, so a
                    # second instantiation declares its own symbols.
                    for nested in re.findall(INCLUDE.pattern, inc_text, re.M):
                        if re.search(r"_externs(_\d+)?\.h$", nested[1]):
                            npath = path.parent / nested[1]
                            if npath.is_file():
                                macros += DEFINE.findall(npath.read_text())
            if TYPE_START.match(strip_comments(chunk).strip()):
                key = norm_ws(chunk)
                for name in defined_type_names(chunk):
                    if name in type_owner and type_owner[name][1] != key:
                        conflicts.append(f"{name}: {type_owner[name][0]} / {unit}")
                    type_owner.setdefault(name, (unit, key))
                if key in seen_types:
                    continue
                seen_types.add(key)
                if not defined_before:
                    types.append(chunk.strip("\n") + "\n")
                    continue
            out.append(chunk)
        body = "".join(out).strip("\n") + "\n"
        undefs: list[str] = []
        for name in macros:
            if name not in undefs:
                undefs.append(name)
        if undefs:
            body += "\n" + "".join(f"#undef {name}\n" for name in undefs)
        bodies.append(body)

    # Prototypes in the folded externs headers of functions that the merged
    # unit defines are replaced by the definition's own signature.
    all_body = "\n".join(bodies)
    defined: dict[str, str] = {}
    for chunk in top_level_chunks(all_body):
        code = strip_comments(chunk).strip()
        if "{" not in code:
            continue
        sig = code.split("{", 1)[0].strip()
        m = re.search(r"(\w+)\s*\([^;{}]*\)$", sig, re.S)
        if m and not sig.startswith(("typedef", "struct", "union", "enum", "#")):
            defined[m.group(1)] = re.sub(r"\s+", " ", sig) + ";"
    kept = []
    for eline in externs:
        m = re.match(r"\s*(?:extern\s+)?[\w\s\*]+?\b(\w+)\s*\([^;]*\)\s*;\s*$", eline)
        if m and m.group(1) in defined:
            proto = defined[m.group(1)]
            if proto not in kept:
                kept.append(proto)
            continue
        kept.append(eline)
    externs = kept

    head = []
    for key, flags in merged_markers.items():
        head.append(f"/* {key}: {' '.join(flags)} */\n")
    common = '#include "common.h"\n'
    if common in hoisted:
        hoisted.remove(common)
        hoisted.insert(0, common)
    head += hoisted
    if types:
        head.append("\n" + "\n".join(types))
    externs_name = f"{new_unit}_externs.h"
    if externs:
        head.append(f'\n#include "{externs_name}"\n')
    source = "".join(head) + "\n" + "\n".join(bodies)
    source = re.sub(r"\n{3,}", "\n\n", source)

    guard = "PE1_EXT_" + re.sub(r"\W", "_", new_unit).upper()
    externs_text = (f"#ifndef {guard}\n#define {guard}\n" + "\n".join(externs) +
                    f"\n#endif /* {guard} */\n") if externs else ""

    for line in conflicts:
        print(f"  type conflict {line}")
    print(f"merge {len(units)} units -> {src_path}/{new_unit}.c at {offset}")
    print("  units: " + " ".join(units))
    print(f"  markers: {merged_markers or 'default'}; externs: {len(externs_files)} headers")
    if args.dry_run:
        sys.stdout.write(source)
        return 0

    # ---- manifest
    new_lines = []
    for idx, line in enumerate(lines):
        m = SUBSEG.match(line)
        if first_idx <= idx <= last_idx:
            if idx == first_idx:
                new_lines.append(re.sub(re.escape(args.first) + r"(?=\s*\])", new_unit, line, 1))
            elif not m:
                new_lines.append(line)  # keep comments
            continue
        if ro_idx and idx in ro_idx:
            if idx == ro_idx[0]:
                new_lines.append(re.sub(re.escape(m.group(3)) + r"(?=\s*\])", new_unit, line, 1))
            continue
        new_lines.append(line)
    cfg.write_text("".join(new_lines))

    # ---- sources
    tracked = git("ls-files").splitlines()
    for unit in units:
        git("rm", "-q", "-f", f"{src_path}/{unit}.c")
    src_dir.mkdir(parents=True, exist_ok=True)
    (src_dir / f"{new_unit}.c").write_text(source)
    if externs_text:
        (src_dir / externs_name).write_text(externs_text)
    for path in externs_files:
        rel = str(path.relative_to(ROOT))
        if rel == f"{src_path}/{externs_name}":
            continue
        users = [p for p in tracked if p.startswith("src/") and p.endswith((".c", ".h", ".inc"))
                 and (ROOT / p).is_file() and path.name in (ROOT / p).read_text()]
        if not users:
            git("rm", "-q", "-f", rel)

    # ---- path mentions in docs and tests
    ov_dir = src_path.split("/")[-1]
    rules = [(re.compile(r"(?<=" + re.escape(ov_dir) + r"/)" + re.escape(u) + r"(\.c)?(?![\w])"),
              new_unit + ".c") for u in units if u != new_unit]
    for path in tracked:
        if any(rx.search(path) for rx in EXCLUDED) or path.startswith(("src/", "configs/")):
            continue
        file = ROOT / path
        if not file.is_file() or file.suffix not in (".md", ".py", ".json", ".txt", ".csv"):
            continue
        try:
            text = file.read_text()
        except UnicodeDecodeError:
            continue
        new = text
        for rx, repl in rules:
            new = rx.sub(repl, new)
        if new != text:
            file.write_text(new)
            print(f"  edit {path}")
    git("add", "-A", src_path, "configs", "docs", "tools/tests")
    return 0


if __name__ == "__main__":
    sys.exit(main())
