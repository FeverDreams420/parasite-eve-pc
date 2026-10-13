#!/usr/bin/env python3
"""Run an overlay Make target concurrently with isolated diagnostic logs."""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import pathlib
import shutil
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[2]
LOG_ROOT = ROOT / "build" / "USA"
SHARED_INPUTS = (
    ROOT / "Makefile",
    ROOT / "requirements.txt",
    ROOT / "configs" / "USA" / "sym.main.txt",
    ROOT / "include",
    # The shared room library and the templates every overlay may include.
    ROOT / "src" / "overlays" / "room_lib",
    ROOT / "tools" / "m2c",
    ROOT / "tools" / "scripts" / "cc.sh",
    ROOT / "tools" / "scripts" / "overlay_link_symbols.py",
    ROOT / "linkers" / "USA" / "overlay_abi.txt",
    ROOT / "linkers" / "USA" / "overlay_windows.txt",
    ROOT / "tools" / "scripts" / "overlay_windows.py",
    ROOT / "tools" / "scripts" / "parallel_overlay_make.py",
)


def main_symbol_table() -> bytes:
    """Main's global symbols and their addresses: overlays link against them,
    so an overlay is stale whenever main moves a symbol it may use."""
    elf = ROOT / "build" / "USA" / "main.elf"
    if not elf.exists():
        return b""
    nm = shutil.which("mipsel-none-elf-nm") or str(
        ROOT / "tools" / "binutils-2.45" / "bin" / "mipsel-none-elf-nm")
    return subprocess.run([nm, "-g", "--defined-only", str(elf)], check=True,
                          stdout=subprocess.PIPE).stdout


def overlay_fingerprint(name: str, main_symbols: bytes = b"") -> str:
    """Hash the inputs that can affect one overlay's linked bytes."""
    inputs = SHARED_INPUTS + (
        ROOT / "configs" / "USA" / "overlays" / (name + ".yaml"),
        ROOT / "configs" / "USA" / "overlays" / ("sym." + name + ".txt"),
        ROOT / "src" / "overlays" / name,
    )
    files: list[pathlib.Path] = []
    for path in inputs:
        if path.is_dir():
            files.extend(item for item in path.rglob("*") if item.is_file())
        elif path.is_file():
            files.append(path)

    digest = hashlib.sha256()
    digest.update(main_symbols)
    digest.update(b"\0")
    for path in sorted(set(files)):
        digest.update(str(path.relative_to(ROOT)).encode())
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--target", default="overlay-build")
    parser.add_argument("overlays", nargs="+")
    args = parser.parse_args()
    LOG_ROOT.mkdir(parents=True, exist_ok=True)
    main_symbols = main_symbol_table() if args.target == "overlay-build" else b""

    def run(name):
        log = LOG_ROOT / ("%s.%s.log" % (args.target, name))
        overlay_dir = LOG_ROOT / "overlays" / name
        stamp = overlay_dir / ".input-sha256"
        fingerprint = overlay_fingerprint(name, main_symbols) if args.target == "overlay-build" else None
        if (fingerprint is not None
                and stamp.exists()
                and stamp.read_text().strip() == fingerprint
                and (overlay_dir / (name + ".bin")).exists()
                and (overlay_dir / (name + ".map")).exists()):
            log.write_text("cached: inputs unchanged\n")
            return name, 0, log, True
        with log.open("w") as output:
            done = subprocess.run(
                ["make", "--no-print-directory", args.target, "OVERLAY=" + name],
                cwd=ROOT, stdout=output, stderr=subprocess.STDOUT,
            )
        if done.returncode == 0 and fingerprint is not None:
            stamp.write_text(fingerprint + "\n")
        return name, done.returncode, log, False

    failed = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for name, code, log, cached in pool.map(run, args.overlays):
            if code:
                failed.append((name, log))
                print("FAIL " + name, flush=True)
            elif cached:
                print("CACHED " + name, flush=True)
            else:
                print("OK   " + name, flush=True)

    for name, log in failed:
        print("\n== %s: %s ==" % (name, log.relative_to(ROOT)))
        print("".join(log.read_text(errors="replace").splitlines(True)[-40:]), end="")
    return int(bool(failed))


if __name__ == "__main__":
    sys.exit(main())
