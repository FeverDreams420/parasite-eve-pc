"""Runs lines that check the randomizer's spots in the mod-randomizer-run test (ctest mod-randomizer-gives):
every table give a run reaches hands out the seed's item, and a spot seen in several rooms is one chest or
pickup there: taking it in one room sets its flag, and with that flag set the other rooms' copies give
nothing. Run in build-mods/mods:

  python -I giveruns.py cands randomizer-spots.txt runs.txt randomizer-run.txt [zone-runs.txt:zone-run.txt ...] > give-cands.txt
  test-mod-randomizer-run give-cands-out.txt give-cands.txt          (PE_TEST_DISC set; seconds)
  python -I giveruns.py pick randomizer-spots.txt give-cands.txt give-cands-out.txt > gives-runs.txt

randomizer-spots.txt comes from mod-randomizer-rooms, runs.txt from runlist.py and randomizer-run.txt is
runall.sh's output for it. cands: per table give, Aya stands on the give's actor (at=@E: the touch that
opens a chest or starts a talk) and presses, at 4 facings, at up to 4 idle places of the room where that
actor is placed (spread over V0x4a) and at V0x4a = 0. Runs and outputs given as pairs (the zone runs of
zoneruns.py, which reach floor pickups by walking into their zones) add the line with the fewest stops that
ran each give there (minutes over the full zone runs). pick: per give, the first candidate that ran it,
with want=; a give whose spot has an opened / taken flag (the flag lines) also gets flag=, and each copy
of the spot in another room a run with that flag set and never=. Each line is written once per seed in
SEEDS. Gives no candidate reached are listed on stderr."""
import sys

SEEDS = ((111111, 0), (222222, 1), (333333, 2))   # Easy, Medium, Hard
FACINGS = (0, 0x400, 0x800, 0xC00)


def table_gives(path):
    """{(room, block offset): (room offset, spot, entity)} and {room offset key: (var, bit)} from randomizer-spots.txt."""
    gives, flags = {}, {}
    for line in open(path):
        p = line.rstrip("\n").split("\t")
        if p[0] == "give" and len(p) > 6 and p[6].startswith("spot="):
            gives[(p[1], int(p[4], 16))] = (int(p[2][1:], 16), int(p[6][5:]), p[3])
        elif p[0] == "flag" and p[4] != "-":
            var, bit = p[3][1:].split("|")
            flags[(p[1], int(p[2][1:], 16))] = (int(var, 16), int(bit, 16))
    return gives, flags


def cands(spots, runs, outs, *pairs):
    gives, _ = table_gives(spots)
    rooms, found = {room for room, _ in gives}, {}
    for pair in pairs:   # the line with the fewest stops, then the lowest V0x4a, that ran each give
        zone_runs, zone_outs = pair.split(":")
        for li, lo in zip(open(zone_runs), open(zone_outs)):
            if "\tgave=+" not in lo or lo.split("\t", 2)[1] not in rooms: continue
            room = lo.split("\t", 2)[1]
            if not li.startswith(room): sys.exit(f"{zone_runs} and {zone_outs} don't pair up: {li.strip()!r}")
            for x in lo.split("\tgave=", 1)[1].split("\t", 1)[0].split(","):
                key, score = (room, int(x[1:], 16)), (li.count("at="), int(li.split()[1], 16))
                if key in gives and (key not in found or score < found[key][0]): found[key] = (score, li.strip())
    places = {}
    for li, lo in zip(open(runs), open(outs)):
        f = lo.rstrip("\n").split("\t")
        if "end=idle" not in f or "battle=1" in f: continue
        alive = next((x[6:] for x in f if x.startswith("alive=")), "").split(",")
        places.setdefault(f[1], []).append((int(li.split()[1], 16), li.strip(), set(alive)))
    for (room, at), (room_at, spot, entity) in sorted(gives.items()):
        ps = sorted({(a, line) for a, line, alive in places.get(room, []) if entity in alive})
        pick = [ps[i * (len(ps) - 1) // 3] for i in range(4)] if len(ps) > 4 else ps
        print(f"# spot {spot} {room} +{room_at:06X} e{entity}")
        if (room, at) in found: print(found[(room, at)][1])
        for line in dict.fromkeys([line for _, line in pick] + [f"{room} 0x0 0x0"]):
            for facing in FACINGS: print(f"{line} keys=all press at=@{entity},{facing}")


def pick(spots, cand_path, out_path):
    gives, flags = table_gives(spots)
    lines = [l.strip() for l in open(cand_path) if l.strip() and not l.startswith("#")]
    outs = [l.rstrip("\n") for l in open(out_path)]
    if len(lines) != len(outs): sys.exit(f"{len(lines)} candidates, {len(outs)} outputs")
    first = {}
    for line, out in zip(lines, outs):
        room = out.split("\t")[1]
        ran = out.split("\tgave=", 1)[1].split("\t", 1)[0] if "\tgave=" in out else ""
        for x in ran.split(","):
            if x and (room, int(x[1:], 16)) in gives: first.setdefault((room, int(x[1:], 16)), line)
    by_spot = {}
    for key, (room_at, spot, _) in gives.items(): by_spot.setdefault(spot, []).append(key)
    checks = []
    for spot, members in sorted(by_spot.items()):
        for key in sorted(members):
            room, at = key
            if key not in first: print(f"no run reaches spot {spot}: {room} +{gives[key][0]:06X}", file=sys.stderr); continue
            flag = flags.get((room, gives[key][0]))
            checks.append(f"{first[key]} want=+{at:X}" + (f" flag=V{flag[0]:#x}|{flag[1]:#x}" if flag else ""))
            if not flag: continue
            for other in sorted(members):
                if other[0] == room or other not in first: continue
                tokens, done = first[other].split(), False
                for i, t in enumerate(tokens):   # the flag on top of the place's own value of that variable
                    if t.startswith("V") and "=" in t and int(t[1:t.index("=")], 16) == flag[0]:
                        tokens[i] = f"V{flag[0]:#x}={int(t.split('=')[1], 16) | flag[1]:#x}"; done = True
                if not done: tokens.insert(3, f"V{flag[0]:#x}={flag[1]:#x}")
                checks.append(f"{' '.join(tokens)} never=+{other[1]:X}")
    print(f"# Generated by mods/pe_randomizer/tools/giveruns.py: {len(first)} of {len(gives)} table gives reached.")
    for seed, difficulty in SEEDS:
        for line in checks: print(f"{line} seed={seed},{difficulty}")


if __name__ == "__main__":
    sys.stdout.reconfigure(newline="\n")
    if len(sys.argv) >= 5 and sys.argv[1] == "cands": cands(*sys.argv[2:])
    elif len(sys.argv) == 5 and sys.argv[1] == "pick": pick(*sys.argv[2:])
    else: sys.exit(__doc__)
