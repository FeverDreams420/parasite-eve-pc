#!/usr/bin/env python3
"""List symbols declared more than once with different types.

Scans the file-scope declarations in include/ and src/ (headers, C sources
and .inc templates): `extern` objects and function prototypes. Two
declarations of one name conflict when their normalized types differ;
parameter names, whitespace, array bounds and `extern` are ignored, so a
prototype repeated with other parameter names is not a conflict. A name
bound to another symbol with `__asm__("...")` is reported under its C name.
D_/func_ names at overlay addresses (0x80100000 and up) name a different
symbol in each overlay, so they conflict only within one overlay (a header
counts towards every overlay whose sources include it). A unit in a shared
overlay directory (room_lib) counts towards each overlay whose splat config
links it, since every room links its own copy at its own addresses.

Usage: decl_conflicts.py [--count] [--name SYM] [--exclude PATH_PREFIX ...]

Prints each conflicting symbol with its distinct types and where each is
declared, then the number of conflicting symbols. --count prints only the
number. Static and inline definitions are not declarations of a shared
symbol and are skipped.
"""
from __future__ import annotations

import argparse
import collections
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCAN_DIRS = ("include", "src")
SUFFIXES = {".h", ".c", ".inc"}

KEYWORDS = {
    "void", "char", "short", "int", "long", "float", "double", "signed",
    "unsigned", "const", "volatile", "struct", "union", "enum", "register",
    "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "f32",
}
IDENT = re.compile(r"[A-Za-z_]\w*")


def strip_comments(text: str) -> str:
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * text.count("\n", i, j))
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def strip_preprocessor(text: str) -> str:
    lines = text.split("\n")
    out = []
    cont = False
    for line in lines:
        if cont or line.lstrip().startswith("#"):
            cont = line.rstrip().endswith("\\")
            out.append("")
        else:
            out.append(line)
    return "\n".join(out)


def top_level_statements(text: str):
    """Yield (line, statement) for each brace-depth-0 statement ending in ';'.

    A statement that opens a brace block at depth 0 after a ')' is a
    function body and is dropped; aggregate definitions keep their braces
    in the statement text.
    """
    depth = 0
    start = 0
    line = 1
    stmt_line = 1
    i, n = 0, len(text)
    skipping_body = False
    paren = 0
    while i < n:
        c = text[i]
        if c == "\n":
            line += 1
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j
        elif c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif c == "{":
            if depth == 0 and paren == 0:
                head = text[start:i].rstrip()
                if head.endswith(")") or re.search(r"\)\s*__attribute__", head):
                    skipping_body = True
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and skipping_body:
                skipping_body = False
                start = i + 1
                stmt_line = line
        elif c == ";" and depth == 0 and paren == 0:
            yield stmt_line, text[start:i]
            start = i + 1
            stmt_line = line
        if not text[start:i + 1].strip():
            stmt_line = line
        i += 1


def split_top(s: str, sep: str = ","):
    parts, depth, cur = [], 0, []
    for c in s:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    parts.append("".join(cur))
    return parts


# common.h's integer typedefs (and m2c's M2C_UNK) name the same types as
# their spellings; a struct tag is compared by name.
ALIASES = [
    (r"\bs8\b", "signed char"), (r"\bu8\b", "unsigned char"),
    (r"\bs16\b", "short"), (r"\bu16\b", "unsigned short"),
    (r"\bs32\b", "int"), (r"\bu32\b", "unsigned int"),
    (r"\bM2C_UNK\b", "int"),
    (r"\bu_char\b", "unsigned char"), (r"\bu_short\b", "unsigned short"),
    (r"\bu_int\b", "unsigned int"), (r"\bu_long\b", "unsigned long"),
    # A tag and its typedef usually share a name (typedef struct X {..} X).
    (r"\b(?:struct|union|enum) ", ""),
    (r"\bsigned (char)\b", "signed char"),
    (r"\bsigned (short|int|long)\b", r"\1"),
    (r"\b(short|long) int\b", r"\1"),
    (r"\bunsigned(?! (?:char|short|int|long)\b)", "unsigned int"),
    (r"\bsigned(?! (?:char|short|int|long)\b)", "int"),
]


def norm_ws(s: str) -> str:
    s = re.sub(r"\s+", " ", s).strip()
    for pat, rep in ALIASES:
        s = re.sub(pat, rep, s)
    s = re.sub(r"\s*([*()\[\],])\s*", r"\1", s)
    return s


def norm_param(p: str) -> str:
    p = norm_ws(p)
    if p in ("", "void", "..."):
        return p
    # Function-pointer parameter: drop the name inside (*name).
    p = re.sub(r"\(\*\s*[A-Za-z_]\w*\s*\)", "(*)", p)
    p = re.sub(r"\[[^\]]*\]", "[]", p)
    m = re.match(r"^(.*?)([A-Za-z_]\w*)((?:\[\])*)$", p)
    if m:
        head, name, arr = m.groups()
        head_s = head.strip()
        if head_s and name not in KEYWORDS and (
                head_s.endswith("*") or IDENT.fullmatch(head_s.split(" ")[-1])):
            # `u8 *x`, `int x`, `struct Foo x`: drop the name.
            if not (head_s in ("struct", "union", "enum")):
                p = head_s + ("*" if arr else "")
                if arr and head_s.endswith("*"):
                    p = head_s + "*"
    p = p.replace("register ", "")
    return norm_ws(p)


DECL_SKIP = re.compile(r"^\s*(typedef|static|inline|__inline|INCLUDE_ASM|INCLUDE_RODATA|PSYQ_ASM_FUNCTION|PSYQ_ASM_OBJECT)\b")
ASM_ALIAS = re.compile(r"\s*(?:__asm__|asm)\s*\(\s*\"[^\"]*\"\s*\)")
ATTR = re.compile(r"__attribute__\s*\(\(.*?\)\)")


MACRO_CALL = re.compile(r"\s*[A-Z][A-Z0-9_]*\s*\(")


def strip_macro_calls(s: str) -> str:
    """Drop file-scope macro invocations that end without a semicolon.

    A body macro (`ROOMLIB_CLOSE_TARGET(name)`) expands to a function
    definition, so nothing ends its statement and the scanner would glue it
    to the next declaration.
    """
    while True:
        m = MACRO_CALL.match(s)
        if not m:
            return s
        depth, i = 0, m.end() - 1
        while i < len(s):
            if s[i] == "(":
                depth += 1
            elif s[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        s = s[i + 1:]


def parse_decls(stmt: str):
    """Yield (name, type) for a file-scope declaration statement."""
    s = strip_macro_calls(stmt).strip()
    if not s or DECL_SKIP.match(s) or "{" in s or "=" in s.split("(")[0]:
        return
    if re.match(r"^(struct|union|enum)\s+\w+\s*$", s):
        return
    if "=" in s:
        return
    s = ASM_ALIAS.sub("", s)
    s = ATTR.sub("", s)
    s = re.sub(r"^\s*extern\s+", "", s)
    s = s.strip()
    # Several declarators after one base type (`int f(int), g(void);`):
    # parse each with the base type.
    m = re.match(r"^((?:(?:const|volatile|signed|unsigned|struct|union|enum)\s+)*[A-Za-z_]\w*)\s+(.*)$", s, re.S)
    if m and m.group(1).split()[-1] not in ("struct", "union", "enum"):
        parts = split_top(m.group(2))
        if len(parts) > 1 and all(p.strip() for p in parts):
            for part in parts:
                yield from parse_decls(f"{m.group(1)} {part.strip()}")
            return
    # Function prototype: <ret> <name>(<params>), possibly returning a pointer.
    m = re.match(r"^(.*?)\b([A-Za-z_]\w*)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*$", s, re.S)
    if m and not re.search(r"\(\s*\*", s.split(m.group(2))[0] + m.group(2)):
        ret, name, params = m.groups()
        if name in KEYWORDS or not ret.strip():
            return
        plist = [norm_param(p) for p in split_top(params)]
        yield name, f"{norm_ws(ret)} ({','.join(plist)})"
        return
    # Object declarations, possibly several declarators.
    m = re.match(r"^((?:(?:const|volatile|signed|unsigned|struct|union|enum)\s+)*[A-Za-z_]\w*)\s*(.*)$", s, re.S)
    if not m:
        return
    base, rest = m.groups()
    if base in ("struct", "union", "enum"):
        return
    for d in split_top(rest):
        d = d.strip()
        if not d:
            continue
        names = IDENT.findall(re.sub(r"\[[^\]]*\]", "", d))
        names = [x for x in names if x not in ("const", "volatile")]
        if not names:
            continue
        name = names[-1] if "(" not in d else names[0]
        if "(" in d:
            # Function pointer object: (*name)(params); take the first ident.
            name = re.search(r"\(\s*\*+\s*(?:const\s+|volatile\s+)*([A-Za-z_]\w*)", d)
            if not name:
                continue
            name = name.group(1)
        declarator = d.replace(name, "", 1)
        declarator = re.sub(r"\[[^\]]*\]", "[]", declarator)
        yield name, norm_ws(f"{base} {declarator}")


def scan(excludes):
    decls = collections.defaultdict(lambda: collections.defaultdict(list))
    for top in SCAN_DIRS:
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            rel = path.relative_to(ROOT).as_posix()
            if any(rel.startswith(e) for e in excludes):
                continue
            text = strip_preprocessor(strip_comments(path.read_text(errors="replace")))
            for line, stmt in top_level_statements(text):
                for name, typ in parse_decls(stmt):
                    decls[name][typ].append(f"{rel}:{line}")
    return decls


# Overlay addresses: the same D_/func_ name in two overlays names two
# different symbols, so such names only conflict within one overlay.
OVERLAY_NAME = re.compile(r"^(?:D|func|jtbl)_(80[0-9A-F]{6})")
OVERLAY_BASE = 0x80100000
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


OVERLAY_CONFIGS = ROOT / "configs" / "USA" / "overlays"
SHARED_UNIT = re.compile(r"\[\s*0x[0-9A-Fa-f]+\s*,\s*[.\w]+\s*,\s*\.\./(\w+)/(\w+)\s*\]")


def shared_unit_links():
    """Map (shared dir, unit) to the overlays whose splat config links it.

    Shared overlay directories (room_lib) hold units that splat links into
    several rooms, each at its own addresses; a unit's overlay-address
    names belong to the rooms that link it, not to the directory.
    """
    links = collections.defaultdict(set)
    for cfg in sorted(OVERLAY_CONFIGS.glob("*.yaml")):
        for shared, unit in SHARED_UNIT.findall(cfg.read_text(errors="replace")):
            links[(shared, unit)].add(cfg.stem)
    return links


def resolve_include(src: pathlib.Path, inc: str):
    """Path of an included file relative to ROOT, or None if not in tree."""
    for base in (src.parent, ROOT / "include"):
        cand = (base / inc).resolve()
        if cand.is_file():
            try:
                return cand.relative_to(ROOT).as_posix()
            except ValueError:
                return None
    return None


def overlay_scopes():
    """Map each scanned file to the overlays its declarations belong to.

    An overlay source belongs to its directory's overlay; a shared-directory
    unit belongs to every overlay that links it; a header or template
    belongs to the overlays of every file including it, directly or
    through other headers.
    """
    links = shared_unit_links()
    shared_dirs = {d for d, _ in links}
    included_by = collections.defaultdict(set)   # file -> files including it
    own = {}
    for top in SCAN_DIRS:
        for path in (ROOT / top).rglob("*"):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            rel = path.relative_to(ROOT).as_posix()
            for inc in INCLUDE.findall(path.read_text(errors="replace")):
                target = resolve_include(path, inc)
                if target:
                    included_by[target].add(rel)
            parts = rel.split("/")
            if parts[0] == "src" and parts[1] == "overlays":
                ov = parts[2]
                if ov in shared_dirs:
                    own[rel] = set(links.get((ov, path.stem), ()))
                else:
                    own[rel] = {ov}
    scopes = {}

    def resolve(rel, seen):
        if rel in scopes:
            return scopes[rel]
        out = set(own.get(rel, ()))
        for parent in included_by.get(rel, ()):
            if parent not in seen:
                out |= resolve(parent, seen | {parent})
        return out

    for rel in set(own) | set(included_by):
        scopes[rel] = resolve(rel, {rel})
    return scopes


def location_scopes(loc, scopes):
    path = loc.rsplit(":", 1)[0]
    if path.startswith("src/overlays/") or path.startswith("include/"):
        return scopes.get(path) or {path}
    return {"main"}


def find_conflicts(decls):
    """Symbols with more than one type, overlay names compared per overlay."""
    scopes = None
    conflicts = {}
    for name, types in decls.items():
        if len(types) < 2:
            continue
        m = OVERLAY_NAME.match(name)
        if m and int(m.group(1), 16) >= OVERLAY_BASE:
            if scopes is None:
                scopes = overlay_scopes()
            per_scope = collections.defaultdict(set)
            for typ, where in types.items():
                for loc in where:
                    for scope in location_scopes(loc, scopes):
                        per_scope[scope].add(typ)
            if not any(len(t) > 1 for t in per_scope.values()):
                continue
        conflicts[name] = types
    return conflicts


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--count", action="store_true", help="print only the count")
    ap.add_argument("--name", action="append", default=[], help="show only these symbols")
    ap.add_argument("--exclude", action="append", default=[],
                    help="skip files under this path prefix (repeatable)")
    args = ap.parse_args()
    decls = scan(args.exclude)
    conflicts = find_conflicts(decls)
    if not args.count:
        for name in sorted(conflicts):
            if args.name and name not in args.name:
                continue
            types = conflicts[name]
            print(f"{name}: {len(types)} types")
            for typ, where in sorted(types.items(), key=lambda kv: -len(kv[1])):
                more = f" (+{len(where) - 3} more)" if len(where) > 3 else ""
                print(f"    {typ}\n        {', '.join(where[:3])}{more}")
    print(len(conflicts))
    return 0


if __name__ == "__main__":
    sys.exit(main())
