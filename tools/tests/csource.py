"""Locate decompiled functions and turn their translation units into host C.

Source files under src/ are merged, split and renamed constantly, so tests
must not hardcode a file name.  Find the unit that defines a symbol instead:

    path = find_definition("func_80197230")
    text = host_unit("func_80197230")            # preprocessed, host-ready
    text = function_text("func_80197230")        # just that definition

host_unit() preprocesses the whole unit with the project include paths and
tools/tests/host_psyq.h, removes PSX-only constructs a 64-bit host compiler
rejects (pointer-size static asserts, fixed-register pins, top-level asm), and
keeps only the requested function definitions plus the static helpers they
reach.  Every other function in the unit is reduced to a prototype (or
dropped when static), so a test only has to stub what its subject calls.
"""
from __future__ import annotations

import functools
import pathlib
import re
import shutil
import subprocess
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
HOST_PSYQ = ROOT / "tools/tests/host_psyq.h"

_IDENT = re.compile(r"[A-Za-z_]\w*")


@functools.lru_cache(maxsize=None)
def _c_files(root: str) -> tuple[pathlib.Path, ...]:
    base = ROOT / root
    return tuple(sorted(p for p in base.rglob("*.c") if p.is_file()))


@functools.lru_cache(maxsize=None)
def _read(path: pathlib.Path) -> str:
    return path.read_text(errors="replace")


def _definition_regex(name: str) -> re.Pattern[str]:
    # Return type and name at the start of a line, parameter list, then the
    # opening brace (possibly on the next line).  Excludes prototypes.
    return re.compile(
        r"^(?!\s)(?![^\n]*\b(?:typedef|extern)\b)[^\n;#=]*?\b"
        + re.escape(name)
        + r"\s*\([^;{}]*\)(?:\s*[^;{}()]+;)*\s*\{",
        re.M,
    )


def find_definitions(name: str, under: str = "src") -> list[pathlib.Path]:
    """All .c files under `under` that define function `name`."""
    pattern = _definition_regex(name)
    needle = name
    return [p for p in _c_files(under)
            if needle in _read(p) and pattern.search(_read(p))]


def find_definition(name: str, under: str = "src") -> pathlib.Path:
    """The single .c file under `under` that defines function `name`."""
    hits = find_definitions(name, under)
    if len(hits) != 1:
        raise LookupError(f"{name}: expected one defining unit under {under}, "
                          f"found {[str(p.relative_to(ROOT)) for p in hits]}")
    return hits[0]


def find_header(pattern: str, under: str = "include") -> pathlib.Path:
    """The single header under `under` whose text matches regex `pattern`."""
    rx = re.compile(pattern, re.M)
    hits = [p for p in sorted((ROOT / under).rglob("*.h")) if rx.search(_read(p))]
    if len(hits) != 1:
        raise LookupError(f"{pattern!r}: expected one header under {under}, "
                          f"found {[str(p.relative_to(ROOT)) for p in hits]}")
    return hits[0]


# --- top-level C splitting -------------------------------------------------

def _skip_literal(text: str, i: int) -> int:
    quote = text[i]
    i += 1
    while i < len(text) and text[i] != quote:
        i += 2 if text[i] == "\\" else 1
    return i + 1


def _match_brace(text: str, i: int) -> int:
    """Index just past the brace block that opens at text[i]."""
    depth = 0
    while i < len(text):
        c = text[i]
        if c in "\"'":
            i = _skip_literal(text, i)
            continue
        if text.startswith("/*", i):
            i = text.index("*/", i + 2) + 2
            continue
        if text.startswith("//", i):
            i = text.find("\n", i)
            i = len(text) if i < 0 else i
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced braces")


# Old-style (K&R) definition head: an identifier-only parameter list followed
# by one or more parameter declarations.
_KR_HEAD = re.compile(
    r"\s*([^;{}()]*?\b\w+\s*\(\s*\w+(?:\s*,\s*\w+)*\s*\))"
    r"(?:\s*[^;{}()]+;)+\s*$"
)


def _strip_comments(text: str) -> str:
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def _is_kr_head(text: str) -> bool:
    return _KR_HEAD.match(_strip_comments(text)) is not None


def _blank(text: str) -> bool:
    """True when `text` holds only whitespace and comments."""
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S).strip() == ""


def _items(text: str):
    """Yield (kind, header, body, start, end) for each top-level item.

    kind is "function" for a definition (body is the brace block) and "other"
    for declarations, typedefs and directives.
    """
    i = start = 0
    paren = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            i = _skip_literal(text, i)
            continue
        if text.startswith("/*", i):
            i = text.index("*/", i + 2) + 2
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
            continue
        if c == "#" and _blank(text[start:i]):
            j = text.find("\n", i)
            while j > 0 and text[j - 1] == "\\":
                j = text.find("\n", j + 1)
            j = n if j < 0 else j + 1
            yield "other", text[start:j], "", start, j
            i = start = j
            continue
        if c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif c == ";" and paren == 0:
            if _is_kr_head(text[start:i + 1]):
                # Old-style definition: parameter declarations precede the body.
                i += 1
                continue
            yield "other", text[start:i + 1], "", start, i + 1
            start = i + 1
        elif c == "{" and paren == 0:
            end = _match_brace(text, i)
            header = text[start:i]
            stripped = re.sub(r"__attribute__\s*\(\(.*?\)\)", "", header).strip()
            is_function = ((stripped.endswith(")") or _is_kr_head(header))
                           and not re.match(r"\s*(typedef|struct|union|enum)\b", stripped)
                           and "=" not in stripped)
            if is_function:
                yield "function", header, text[i:end], start, end
                i = start = end
                continue
            # Aggregate definition or initializer: continue to the ';'.
            i = end
            continue
        i += 1
    if text[start:].strip():
        yield "other", text[start:], "", start, n


def _function_name(header: str) -> str | None:
    header = re.sub(r"__attribute__\s*\(\(.*?\)\)", "", header)
    kr = _KR_HEAD.match(_strip_comments(header))
    if kr:
        header = kr.group(1)
    m = re.search(r"(\w+)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*$", header.strip())
    return m.group(1) if m else None


def function_text(name: str, path: pathlib.Path | None = None) -> str:
    """Raw source text of one function definition (no preprocessing)."""
    path = path or find_definition(name)
    for kind, header, body, _, _ in _items(_read(path)):
        if kind == "function" and _function_name(header) == name:
            return header.lstrip("\n") + body + "\n"
    raise LookupError(f"{name} not defined in {path}")


def functions_text(*names: str) -> str:
    """Concatenated raw definitions of several functions, wherever they live."""
    return "".join(function_text(name) for name in names)


def function_closure(name: str, path: pathlib.Path | None = None) -> str:
    """Raw text of `name` plus the static functions of its unit it reaches.

    Use this for "this function is plain C" checks: static inline helpers
    that the function calls are part of what it compiles to.
    """
    path = path or find_definition(name)
    return text_function_closure(_read(path), name, path)


def text_function_closure(text: str, name: str, origin=None) -> str:
    """function_closure over C text, e.g. a unit's preprocessed output."""
    texts = {}
    for kind, header, body, _, _ in _items(text):
        if kind == "function":
            texts[_function_name(header)] = (header, body)
    if name not in texts:
        raise LookupError(f"{name} not defined in {origin or 'text'}")
    keep, work = [name], [name]
    while work:
        body = texts[work.pop()][1]
        for ident in sorted(set(_IDENT.findall(body))):
            if ident in texts and ident not in keep \
                    and re.search(r"\bstatic\b", texts[ident][0]):
                keep.append(ident)
                work.append(ident)
    return "".join(texts[n][0].lstrip("\n") + texts[n][1] + "\n" for n in keep)


def defined_functions(path: pathlib.Path) -> list[str]:
    return [_function_name(h) for k, h, _, _, _ in _items(_read(path))
            if k == "function"]


# --- host conversion -------------------------------------------------------

_STATIC_ASSERT = re.compile(
    r"typedef\s+char\s+\w+\s*\[[^;]*?\?\s*1\s*:\s*-\s*1\s*\]\s*;")
_REGISTER_PIN = re.compile(r"\b(?:asm|__asm__)\s*\(\s*\"\$?\w+\"\s*\)")
_TOP_ASM = re.compile(r"^\s*(?:asm|__asm__)\b")
# ELF section placement (e.g. PSY-Q padding words) is invalid on Mach-O hosts.
_SECTION_ATTR = re.compile(r"__attribute__\s*\(\(\s*section\s*\([^()]*\)\s*\)\)")
# Empty-template scheduling barriers may name MIPS-only constraints ("=l" for
# LO, "=h" for HI) or clobber MIPS registers ("$2"); neither exists on the
# host, where the barrier only needs to stay a no-op.
_EMPTY_ASM = re.compile(r"\b(?:asm|__asm__)\s*(?:volatile|__volatile__)?\s*\(\s*\"\"\s*:")
_MIPS_CLOBBER = re.compile(r"\"\$\w+\"\s*,?\s*")


def _host_barriers(text: str) -> str:
    out = []
    pos = 0
    for m in _EMPTY_ASM.finditer(text):
        if m.start() < pos:
            continue
        depth = 0
        i = m.end() - 1
        while i < len(text):
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
                if depth < 0:
                    break
            i += 1
        stmt = text[m.start():i]
        stmt = re.sub(r"\"=[lh]\"", "\"=r\"", stmt)
        stmt = _MIPS_CLOBBER.sub("", stmt)
        stmt = re.sub(r",\s*$", "", stmt)
        out.append(text[pos:m.start()])
        out.append(stmt)
        pos = i
    out.append(text[pos:])
    return "".join(out)


def preprocess(path: pathlib.Path, prefix: str = "", include_dirs=()) -> str:
    """Host-preprocess a unit with project include paths and host_psyq.h."""
    args = ["cc", "-E", "-P", "-std=gnu11", "-DPERMUTER",
            "-include", str(HOST_PSYQ),
            "-I", str(ROOT / "include"), "-I", str(path.parent)]
    for d in include_dirs:
        args += ["-I", str(d)]
    source = prefix + "\n#line 1 \"" + str(path) + "\"\n" + _read(path)
    result = subprocess.run(args + ["-x", "c", "-"], input=source, text=True,
                            capture_output=True, cwd=path.parent)
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    return result.stdout


def _identifiers(text: str) -> set[str]:
    return set(_IDENT.findall(text))


def host_unit(*names: str, path: pathlib.Path | None = None, prefix: str = "",
              keep_all: bool = False, include_dirs=(), under: str = "src") -> str:
    """Preprocessed host-ready C for the unit that defines `names[0]`.

    Only the definitions in `names` (and static functions they reach) are kept
    unless `keep_all`; other non-static functions become prototypes.
    `prefix` is inserted before the unit during preprocessing (for #defines).
    Pass `under` (an overlay directory) for address-named overlay functions:
    the same func_ address can be defined by several overlays.
    """
    path = path or find_definition(names[0], under)
    text = preprocess(path, prefix, include_dirs)
    text = _HOST_U_LONG.sub("typedef unsigned int u_long;", text)
    text = _STATIC_ASSERT.sub("", text)
    text = _REGISTER_PIN.sub("", text)
    text = _SECTION_ATTR.sub("", text)
    text = _host_barriers(text)

    items = list(_items(text))
    functions = {}
    for kind, header, body, _, _ in items:
        if kind == "function":
            functions[_function_name(header)] = (header, body)
    missing = [n for n in names if n not in functions]
    if missing:
        raise LookupError(f"{missing} not defined in {path}")

    keep = set(functions) if keep_all else set(names)
    work = list(keep)
    while work:
        header, body = functions[work.pop()]
        for ident in _identifiers(body):
            if ident in functions and ident not in keep \
                    and re.search(r"\bstatic\b", functions[ident][0]):
                keep.add(ident)
                work.append(ident)

    out = []
    for kind, header, body, _, _ in items:
        if kind == "function":
            name = _function_name(header)
            if name in keep:
                out.append(header + body)
            elif not re.search(r"\bstatic\b", header):
                out.append(header.rstrip() + ";")
        elif _TOP_ASM.match(header):
            continue
        elif re.match(r"\s*register\b", header):
            # A file-scope global register variable lost its asm("$N") pin
            # above; on the host it is just a private global.
            out.append(re.sub(r"\bregister\b", "static", header, count=1))
        else:
            out.append(header)
    return "".join(out) + "\n"


def require_psx_tools() -> None:
    """Skip the calling test when the PSX cross toolchain is not installed.

    cc.sh fails silently without mipsel-none-elf-as on PATH (the build needs
    it too, e.g. tools/binutils-2.45/bin), so report it as an environment
    skip rather than an opaque compile failure.
    """
    missing = [tool for tool in ("mipsel-none-elf-as", "mipsel-none-elf-ld")
               if shutil.which(tool) is None]
    if missing or not (ROOT / "tools/psyq-gcc-2.7.2/cpp").exists():
        raise unittest.SkipTest("PSX cross toolchain unavailable: "
                                + ", ".join(missing or ["tools/psyq-gcc-2.7.2"]))


def psx_compile(source: str, out: pathlib.Path, like: pathlib.Path,
                *cflags: str) -> subprocess.CompletedProcess:
    """Compile `source` text with tools/scripts/cc.sh as if it were `like`.

    The text is written next to `out`, so quoted includes of headers that
    live beside the original unit are resolved through an extra -I.
    """
    require_psx_tools()
    path = out.with_suffix(".c")
    path.write_text(source)
    return subprocess.run(
        [str(ROOT / "tools/scripts/cc.sh"), str(path), str(out),
         "-I" + str(like.parent), *cflags],
        cwd=ROOT, capture_output=True, text=True, errors="replace")


# PSX `long` is 32 bits; on an LP64 host the PSY-Q `u_long` packet words
# would double in size and break every DR_ENV/OT layout a test inspects.
_HOST_U_LONG = re.compile(r"typedef\s+unsigned\s+long\s+u_long\s*;")


def host_source(source: str, include_dirs=(), cwd: pathlib.Path | None = None) -> str:
    """Preprocess arbitrary harness text for the host like host_unit does.

    For harnesses that #include project headers directly: applies the
    project include path, host_psyq.h, PSX-width u_long, and drops the
    pointer-size static asserts, register pins and MIPS-only barriers a
    64-bit host compiler rejects.  System headers may be included too.
    """
    args = ["cc", "-E", "-P", "-std=gnu11", "-DPERMUTER",
            "-include", str(HOST_PSYQ),
            "-I", str(ROOT / "include"), "-I", str(ROOT)]
    for d in include_dirs:
        args += ["-I", str(d)]
    result = subprocess.run(args + ["-x", "c", "-"], input=source, text=True,
                            capture_output=True, cwd=cwd or ROOT)
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    text = _HOST_U_LONG.sub("typedef unsigned int u_long;", result.stdout)
    text = _STATIC_ASSERT.sub("", text)
    text = _REGISTER_PIN.sub("", text)
    text = _SECTION_ATTR.sub("", text)
    return _host_barriers(text)


def object_symbols(obj: pathlib.Path) -> dict[str, int]:
    """Defined .text symbol offsets in a relocatable object (nm)."""
    out = subprocess.run(["mipsel-none-elf-nm", "--defined-only", str(obj)],
                         check=True, capture_output=True, text=True).stdout
    symbols = {}
    for line in out.splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[1] in "tT":
            symbols[fields[2]] = int(fields[0], 16)
    return symbols


@functools.lru_cache(maxsize=None)
def main_symbol_addresses() -> dict[str, int]:
    """Retail addresses of main.exe symbols from configs/ and linkers/."""
    files = [ROOT / "configs/USA/sym.main.txt",
             *sorted((ROOT / "linkers/USA").glob("undefined_*.txt"))]
    rx = re.compile(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", re.M)
    known = {}
    for path in files:
        if path.is_file():
            for name, value in rx.findall(path.read_text()):
                known.setdefault(name, int(value, 16))
    gp = re.search(r"^\s*gp_value:\s*(0x[0-9A-Fa-f]+)",
                   (ROOT / "configs/USA/main.yaml").read_text(), re.M)
    if gp:
        known.setdefault("_gp", int(gp.group(1), 16))
    return known


def _link_unit(name: str, address: int, symbols: str, work: pathlib.Path,
               path: pathlib.Path | None, cflags) -> tuple[bytes, int]:
    """Linked .text of the unit defining `name`, placed so `name` is at
    `address`; returns (text, offset of `name` within it)."""
    require_psx_tools()
    path = path or find_definition(name)
    obj, elf, data = work / "unit.o", work / "unit.elf", work / "unit.bin"
    result = subprocess.run([str(ROOT / "tools/scripts/cc.sh"), str(path),
                             str(obj), *cflags], cwd=ROOT,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(result.stdout + result.stderr)
    offset = object_symbols(obj)[name]
    base = address - offset
    # Unresolved references take their retail address from the project
    # symbol lists (or the address a D_/func_ name encodes); anything left is
    # parked at the base so jal/hi16 relocations fit.
    # Small-data COMMON symbols (MASPSX --use-comm-section) are external
    # data as well; give them their retail address instead of allocating them.
    listing = subprocess.run(["mipsel-none-elf-nm", str(obj)], check=True,
                             capture_output=True, text=True).stdout
    undefined = [(f[-1], f[-2]) for f in map(str.split, listing.splitlines())
                 if len(f) >= 2 and f[-2] in ("U", "C")]
    pinned = set(re.findall(r"(\w+)\s*=", symbols))
    provided = dict(re.findall(
        r"PROVIDE\(\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*\)", symbols))
    known = main_symbol_addresses()
    provide = ""
    if "_gp" in known:
        # The MIPS backend defines _gp itself, so a PROVIDE never applies
        # once the unit has small data; assign the retail value directly.
        provide += f"_gp = 0x{known['_gp']:x};\n"
    for u, kind in undefined:
        encoded = re.fullmatch(r"(?:D|func)_([0-9A-Fa-f]{8})", u)
        value = known.get(u, int(encoded.group(1), 16) if encoded else base)
        if kind == "C":
            # PROVIDE never overrides a COMMON definition; assign directly,
            # preferring an address the caller offered through PROVIDE.
            if u in provided:
                provide += f"{u} = {provided[u]};\n"
            elif u not in pinned:
                provide += f"{u} = 0x{value:x};\n"
        else:
            provide += f"PROVIDE({u} = 0x{value:x});\n"
    # Small data the unit defines itself (a merged unit can own .sdata) sits
    # at its retail address, so $gp-relative offsets come out as in retail.
    small = ""
    out = subprocess.run(["mipsel-none-elf-objdump", "-t", str(obj)], check=True,
                         capture_output=True, text=True).stdout
    for section in (".sdata", ".sbss"):
        for line in out.splitlines():
            fields = line.split()
            if len(fields) >= 5 and fields[-3] == section and fields[1] in ("g", "l") \
                    and not fields[-1].startswith("."):
                sym = fields[-1]
                encoded = re.fullmatch(r"D_([0-9A-Fa-f]{8})", sym)
                where = known.get(sym, int(encoded.group(1), 16) if encoded else None)
                if where is not None:
                    small += " %s 0x%x : { *(%s) }" % (section, where - int(fields[0], 16),
                                                      section)
                    break
    script = work / "unit.ld"
    script.write_text(
        symbols + "\n" + provide
        + "SECTIONS { .text 0x%x : SUBALIGN(4) { *(.text .text.*) }%s "
        "/DISCARD/ : { *(.reginfo) *(.mdebug) } }\n" % (base, small))
    result = subprocess.run(["mipsel-none-elf-ld", "-T", str(script),
                             "--no-warn-mismatch", str(obj), "-o", str(elf)],
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    subprocess.run(["mipsel-none-elf-objcopy", "-O", "binary",
                    "--only-section=.text", str(elf), str(data)],
                   check=True, capture_output=True)
    return data.read_bytes(), offset


def linked_function_bytes(name: str, address: int, size: int, symbols: str,
                          work: pathlib.Path, path: pathlib.Path | None = None,
                          *cflags: str) -> bytes:
    """Compile the unit defining `name` and return its `size` linked bytes.

    Units are merged and split over time, so the object may hold more than
    the function under test.  The whole .text is linked so that `name` lands
    at `address` (in-unit calls therefore resolve to their real neighbours),
    `symbols` (linker-script assignments such as "VSync = 0x80073A44;")
    pins external references (the rest come from main_symbol_addresses),
    and only [address, address + size) is returned.
    """
    text, offset = _link_unit(name, address, symbols, work, path, cflags)
    return text[offset:offset + size]


def linked_range_bytes(address: int, size: int, symbols: str,
                       work: pathlib.Path) -> bytes:
    """Linked bytes of main.exe [address, address + size) from current units.

    The range may span several functions in any number of units; each step
    links the unit defining the function that starts at the cursor (named in
    configs/USA/sym.main.txt) and takes the rest of that unit's text.
    """
    by_address = {}
    for symbol, value in main_symbol_addresses().items():
        by_address.setdefault(value, []).append(symbol)
    out = b""
    cursor = address
    while len(out) < size:
        units = [(n, find_definitions(n)) for n in by_address.get(cursor, [])]
        units = [(n, hits[0]) for n, hits in units if len(hits) == 1]
        if not units:
            raise LookupError(f"no C unit defines a function at 0x{cursor:08X}")
        name, path = units[0]
        text, offset = _link_unit(name, cursor, symbols, work, path, ())
        piece = text[offset:offset + size - len(out)]
        if not piece:
            raise LookupError(f"{name} at 0x{cursor:08X} links no bytes")
        out += piece
        cursor += len(piece)
    return out
