#!/usr/bin/env python3
"""Boot a disc image in an emulator and play the opening with a fixed script.

The emulator is DuckStation's regression-test runner with the AI control
port and the GDB server (`duckstation-regtest -ai-port P -gdb Q
-start-paused`), not part of this repository: pass it with `--emulator` (and
its library directory with `--lib`, exported as DYLD_FALLBACK_LIBRARY_PATH
and LD_LIBRARY_PATH). It finds its BIOS in its own data directory (a
`portable.txt` next to the binary makes that `bios/` beside it; a US BIOS such
as SCPH1001.BIN), and the macOS build loads libshaderc from `../Frameworks`
relative to the binary's directory (a link to the library directory will do).
The runner uses the software renderer and fast boot. Emulation is
deterministic: the same disc and the same inputs on the same frames give the
same frames, so two discs that hold the same game reach the same scene on the
same frame.

The script (STEPS) presses through the intro to the title menu, starts a New
Game, watches the opening FMV and the first scene in front of Carnegie Hall,
accepts the default name, walks Aya through the first playable room into the
next two rooms (the aisle and the lobby), enters the concert hall and plays
the opera scene to the fire. At every checkpoint it saves a screenshot
(PNG) and reads RAM probes over GDB: the scene token (`g_SceneDispatchToken`,
the base-32 room name the dispatcher runs), the vsync counter, the program
counter and the function it lies in. Symbols are resolved from `--elf`, the
main ELF the disc was built from (a shifted build has its own addresses).

    emu_boot_test.py --emulator .../duckstation-regtest --lib .../lib \\
        --out /scratch/retail "disc/Parasite Eve (Disc 1).cue"
    emu_boot_test.py ... --out /scratch/pad10 --elf /scratch/pad10/main.shifted.elf \\
        --compare /scratch/retail/report.json /scratch/pad10/"Parasite Eve (Disc 1).cue"

`--compare` checks the new run against an earlier report: the scene token
must be equal at every checkpoint and the screenshots are compared pixel by
pixel (the share of pixels that differ is reported; 0 means the same frame).
The exit status is 1 when a token differs, a checkpoint is missing or the
emulator stopped presenting frames (a crashed game: the checkpoint keeps the
last picture and the program counter shows where it hangs). `--log-level Dev`
makes the emulator log every CD-ROM command (Setloc/ReadN/ReadS), which shows
which file a run reads where two runs part.
Everything is written to `--out`; nothing in the repository changes.
"""
from __future__ import annotations

import argparse
import json
import os
import socket
import struct
import subprocess
import sys
import time
import zlib
from bisect import bisect_right
from pathlib import Path

FRAMES = 2_000_000  # the runner's frame budget; the script uses far fewer

# (action, argument): wait N frames, press BUTTON (held for HOLD frames, then
# released), hold BUTTON for N frames, shot NAME (a checkpoint). Directions
# are the left stick: the runner's pad is an analog controller whose stick
# drives the d-pad in digital mode, while its d-pad buttons do not reach
# the game.
HOLD = 6
STEPS: list[tuple[str, object]] = [
    ("wait", 1200), ("shot", "publisher"),          # "Published by Square EA"
    ("wait", 1300), ("press", "Start"),             # skip the intro FMV
    ("wait", 180), ("shot", "title"),               # Press Start Button
    ("press", "Start"), ("wait", 300), ("shot", "title_menu"),
    ("press", "LUp"), ("wait", 60), ("shot", "new_game"),
    ("press", "Cross"),                             # New Game: opening FMV
    ("wait", 2400), ("shot", "opening_fmv"),
    ("wait", 2400), ("shot", "carnegie_fmv"),
    ("wait", 2400), ("shot", "carnegie_street"),    # first scene, in engine
    ("wait", 1200),
] + [step for _ in range(4) for step in (("press", "Cross"), ("wait", 120))] + [
    ("shot", "name_entry"),                         # the cursor types A..
] + [step for _ in range(12) for step in (("press", "Cross"), ("wait", 120))] + [
    ("press", "Start"), ("wait", 30),               # .. until full; Start: END
] + [step for _ in range(4) for step in (("press", "Cross"), ("wait", 150))] + [
    ("shot", "carnegie_entrance"),
] + [step for _ in range(8) for step in (("press", "Cross"), ("wait", 150))] + [
    ("wait", 600), ("shot", "first_control"),       # the first playable room
    ("hold", ("LUp", 160)), ("wait", 600), ("shot", "aisle"),
    ("hold", ("LUp", 200)), ("wait", 600), ("shot", "lobby"),
    ("hold", ("LUp", 150)), ("wait", 900), ("shot", "concert_hall"),
] + [step for _ in range(24) for step in (("press", "Cross"), ("wait", 250))] + [
    ("shot", "opera"),
] + [step for _ in range(32) for step in (("press", "Cross"), ("wait", 250))] + [
    ("shot", "fire"),
    ("hold", ("LUp", 600)), ("wait", 300), ("shot", "stage"),
]

PROBES = (("g_SceneDispatchToken", 4), ("g_VSyncCount", 4))


# --- emulator control ----------------------------------------------------------

class Control:
    """The runner's AI port: a line protocol."""

    def __init__(self, port: int, timeout: float = 120):
        deadline = time.time() + timeout
        while True:
            try:
                self.sock = socket.create_connection(("127.0.0.1", port), timeout=timeout)
                break
            except OSError:
                if time.time() > deadline:
                    raise
                time.sleep(0.2)
        self.file = self.sock.makefile("rb")
        self.file.readline()  # greeting

    def command(self, text: str) -> str:
        self.sock.sendall((text + "\n").encode())
        reply = self.file.readline().decode().strip()
        if reply.startswith("ERR"):
            raise RuntimeError(f"{text}: {reply}")
        return reply

    def frame(self) -> int:
        status = self.command("status")
        return FRAMES - int(status.split("frames_remaining=")[1].split()[0])

    def advance(self, frames: int) -> None:
        self.command(f"advance {frames}")
        while "advance_remaining=0" not in self.command("status"):
            time.sleep(0.01)

    def screenshot(self) -> tuple[int, int, bytes, int] | None:
        """The frame the emulator presented last, once it caught up with the
        emulated frame; when it does not within a few seconds (the game no
        longer presents frames, e.g. after a crash), the last one it has, or
        None when it never presented one."""
        want = self.frame()
        last = None
        for _ in range(300):
            self.sock.sendall(b"screenshot\n")
            fields = self.file.readline().decode().split()
            if fields[0] != "SCREENSHOT":
                return last
            width, height, pitch, size, frame = map(int, fields[1:])
            data = self.file.read(size)
            last = width, height, b"".join(data[y * pitch:y * pitch + width * 4] for y in range(height)), frame
            if frame >= want:
                break
            time.sleep(0.01)
        return last


class Gdb:
    """Minimal GDB remote protocol client: memory and registers."""

    def __init__(self, port: int, timeout: float = 30):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=timeout)
        self.buf = b""

    def _packet(self) -> str:
        while True:
            while b"#" not in self.buf or len(self.buf) < self.buf.index(b"#") + 3:
                self.buf += self.sock.recv(65536)
            start = self.buf.find(b"$")
            end = self.buf.index(b"#", start)
            data = self.buf[start + 1:end]
            self.buf = self.buf[end + 3:]
            self.sock.sendall(b"+")
            return data.decode("latin1")

    def command(self, text: str, length: int) -> str:
        """Send TEXT; return the first reply of LENGTH hex digits. The server
        sends a stop notification (S05) each time the runner pauses, so one
        is queued per advance since the last command; they are skipped."""
        body = text.encode("latin1")
        self.sock.sendall(b"$" + body + b"#" + b"%02x" % (sum(body) & 0xFF))
        seen: list[str] = []
        for _ in range(100000):
            reply = self._packet()
            if not seen or seen[-1] != reply[:40]:
                seen.append(reply[:40])
            if reply.startswith("E") and len(reply) == 3:
                raise RuntimeError(f"{text}: {reply}")
            if len(reply) >= length and all(c in "0123456789abcdefABCDEF" for c in reply):
                return reply
        raise RuntimeError(f"{text}: no reply, got {seen}")

    def memory(self, address: int, size: int) -> bytes:
        return bytes.fromhex(self.command(f"m{address:x},{size:x}", 2 * size))

    def registers(self) -> list[int]:
        reply = self.command("g", 38 * 8)
        return [int.from_bytes(bytes.fromhex(reply[i:i + 8]), "little") for i in range(0, len(reply) - 7, 8)]


# --- symbols and images ----------------------------------------------------------

class Symbols:
    def __init__(self, elf: Path):
        from elftools.elf.elffile import ELFFile

        with open(elf, "rb") as fh:
            symtab = ELFFile(fh).get_section_by_name(".symtab")
            self.by_name = {s.name: s["st_value"] for s in symtab.iter_symbols() if s.name}
            funcs = sorted((s["st_value"], s.name) for s in symtab.iter_symbols()
                           if s.name and s["st_info"]["type"] == "STT_FUNC")
        self.func_addrs = [a for a, _ in funcs]
        self.func_names = [n for _, n in funcs]

    def function_at(self, address: int) -> str:
        i = bisect_right(self.func_addrs, address) - 1
        if i < 0:
            return "?"
        return f"{self.func_names[i]}+0x{address - self.func_addrs[i]:X}"


def write_png(path: Path, width: int, height: int, rgba: bytes) -> None:
    raw = b"".join(b"\0" + rgba[y * width * 4:(y + 1) * width * 4] for y in range(height))

    def chunk(kind: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + kind + data
                + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))

    path.write_bytes(b"\x89PNG\r\n\x1a\n"
                     + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
                     + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


def read_png(path: Path) -> tuple[int, int, bytes]:
    """Reads the PNGs write_png writes (RGBA, filter 0 on every row)."""
    data = path.read_bytes()
    at, idat = 8, b""
    width = height = 0
    while at < len(data):
        size = struct.unpack_from(">I", data, at)[0]
        kind = data[at + 4:at + 8]
        body = data[at + 8:at + 8 + size]
        if kind == b"IHDR":
            width, height = struct.unpack_from(">II", body)
        elif kind == b"IDAT":
            idat += body
        at += 12 + size
    raw = zlib.decompress(idat)
    stride = width * 4 + 1
    return width, height, b"".join(raw[y * stride + 1:(y + 1) * stride] for y in range(height))


def pixel_difference(a: Path, b: Path) -> float | None:
    wa, ha, pa = read_png(a)
    wb, hb, pb = read_png(b)
    if (wa, ha) != (wb, hb):
        return None
    differ = sum(pa[i:i + 3] != pb[i:i + 3] for i in range(0, len(pa), 4))
    return differ / (wa * ha)


# --- run -------------------------------------------------------------------------

def free_port() -> int:
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def run(args: argparse.Namespace) -> dict:
    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    symbols = Symbols(args.elf)
    ai_port, gdb_port = free_port(), free_port()
    env = dict(os.environ)
    if args.lib:
        for var in ("DYLD_FALLBACK_LIBRARY_PATH", "LD_LIBRARY_PATH"):
            env[var] = str(args.lib) + (os.pathsep + env[var] if env.get(var) else "")
    log = open(out / "emulator.log", "w")
    proc = subprocess.Popen(
        [str(args.emulator), "-frames", str(FRAMES), "-ai-port", str(ai_port), "-gdb", str(gdb_port),
         "-start-paused", "-console", "-log", args.log_level, "--", str(args.disc.resolve())],
        cwd=args.emulator.parent, env=env, stdout=log, stderr=subprocess.STDOUT)
    report = {"disc": str(args.disc), "elf": str(args.elf), "checkpoints": []}
    try:
        control = Control(ai_port)
        gdb = Gdb(gdb_port)
        for action, arg in STEPS:
            if proc.poll() is not None:
                report["error"] = f"the emulator exited ({proc.returncode})"
                break
            if action == "wait":
                control.advance(arg)
            elif action == "press":
                control.command(f"buttons {arg}=1")
                control.advance(HOLD)
                control.command("release")
            elif action == "hold":
                button, frames = arg
                control.command(f"buttons {button}=1")
                control.advance(frames)
                control.command("release")
            elif action == "shot":
                shot = control.screenshot()
                png = out / f"{len(report['checkpoints']):02d}_{arg}.png"
                if shot is None:
                    width = height = frame = 0
                else:
                    width, height, rgba, frame = shot
                    write_png(png, width, height, rgba)
                probes = {}
                for name, size in PROBES:
                    if name in symbols.by_name:
                        probes[name] = gdb.memory(symbols.by_name[name], size).hex()
                pc = gdb.registers()[37]
                checkpoint = {"name": arg, "frame": control.frame(), "presented": frame,
                              "screenshot": png.name, "size": [width, height], "probes": probes,
                              "pc": f"0x{pc:08X}", "function": symbols.function_at(pc)}
                report["checkpoints"].append(checkpoint)
                print(f"{checkpoint['frame']:6} {arg:18} token {probes.get('g_SceneDispatchToken')} "
                      f"pc {checkpoint['pc']} {checkpoint['function']}", flush=True)
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
        log.close()
    return report


def compare(report: dict, base_path: Path, out: Path) -> list[str]:
    base = json.loads(base_path.read_text())
    problems = []
    base_by_name = {c["name"]: c for c in base["checkpoints"]}
    for c in report["checkpoints"]:
        b = base_by_name.get(c["name"])
        if b is None:
            continue
        shot = out / c["screenshot"]
        diff = pixel_difference(base_path.parent / b["screenshot"], shot) if shot.exists() else None
        c["compare"] = {"pixels_differ": diff,
                        "token_equal": c["probes"].get("g_SceneDispatchToken")
                        == b["probes"].get("g_SceneDispatchToken"),
                        "base_function": b["function"]}
        if not c["compare"]["token_equal"]:
            problems.append(f"{c['name']}: scene token {c['probes'].get('g_SceneDispatchToken')}, "
                            f"base {b['probes'].get('g_SceneDispatchToken')}")
        if c["presented"] + 2 < c["frame"]:
            problems.append(f"{c['name']}: the last frame presented is {c['presented']}, "
                            f"emulated {c['frame']} (no picture since)")
        shown = "no picture or size differs" if diff is None else f"{100 * diff:.2f}% of pixels differ"
        print(f"{c['name']:18} {shown}; token {'equal' if c['compare']['token_equal'] else 'DIFFERS'}")
    missing = [n for n in base_by_name if n not in {c["name"] for c in report["checkpoints"]}]
    if missing:
        problems.append(f"checkpoints not reached: {', '.join(missing)}")
    return problems


def main(argv=None) -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("disc", type=Path, help="the disc's .cue")
    parser.add_argument("--emulator", type=Path, required=True, help="duckstation-regtest with -ai-port/-gdb")
    parser.add_argument("--lib", type=Path, help="the emulator's shared library directory")
    parser.add_argument("--out", type=Path, required=True, help="directory for screenshots and the report")
    parser.add_argument("--elf", type=Path, default=root / "build" / "USA" / "main.elf",
                        help="main ELF of the disc's executable, for the RAM probes")
    parser.add_argument("--compare", type=Path, help="report.json of an earlier run to compare with")
    parser.add_argument("--log-level", default="Info",
                        help="emulator log level for emulator.log (Dev logs every CD-ROM command)")
    args = parser.parse_args(argv)
    report = run(args)
    problems = []
    if args.compare:
        problems = compare(report, args.compare, args.out)
    if report.get("error"):
        problems.append(report["error"])
    report["problems"] = problems
    (args.out / "report.json").write_text(json.dumps(report, indent=1))
    for problem in problems:
        print(f"problem: {problem}")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
