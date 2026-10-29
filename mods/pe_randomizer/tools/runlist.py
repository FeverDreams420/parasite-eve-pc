"""Runs file for the mod-randomizer-run test from storywalk.py's places: one line per place (room,
V0x4a, V0x1) with the story bits that are surely set there.
Usage: python -I runlist.py <walk-states.txt> > runs.txt
Then: test-mod-randomizer-run <randomizer-run.txt> runs.txt (PE_TEST_DISC set; headless by itself)."""
import sys


def main():
    if len(sys.argv) < 2: sys.exit(__doc__)
    seen = set()
    for line in open(sys.argv[1]):
        f = line.rstrip("\n").split("\t")
        room, area, left = f[1], f[2].split("=")[1], f[3].split("=")[1]
        sure = []
        for b in f[4].split():
            var, masks = b.split("=")
            one, zero = (int(x, 16) for x in masks.split("/"))
            if one & ~zero: sure.append(f"{var}={one & ~zero:#x}")
        run = f"{room} {area} {left}" + "".join(" " + s for s in sure)
        if run not in seen:
            seen.add(run); print(run)


if __name__ == "__main__":
    main()
