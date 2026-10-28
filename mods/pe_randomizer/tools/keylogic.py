"""Which keys each spot needs, from real play: python -I keylogic.py pe_play_log.csv [...] > needs.txt
Then: python -I spottable.py <disc.bin> needs.txt > pe_rand_spots.h

The randomizer mod writes a rand_give row each time a spot of its table gives its item (on or off).
A spot's needs are the keys given before the player first reached it in a run: they did reach it
holding those keys (and having reached those keys' spots), so it needs at most those. That's an
over-estimate, never an under-estimate, which is the safe side (a key is only placed where it's surely reachable). Several runs: each spot
keeps the keys in common, which is still an over-estimate. Spots no run reached stay unknown and
never take a key.

It also writes, per spot, the keys whose lock (a rand_lock row: the game first asking whether Aya has
the key) came after the spot was first reached: before=. A key only goes to a spot listed for it, so
it never lands somewhere a player may already have walked past for good. Runs in common, again.
Runs where the randomizer moved keys (rand_layout keys_moved > 0) are left out: there, holding a key
and having reached its spot are two different things.

A run starts at a rand_newgame row. Loading this run's own saves is fine (keys from later in the run count
too: over-estimate again). A save with more play time than the run has had is from another game: the
run stops there, since keys used up in that game (fuses, cardkeys) wouldn't show."""
import collections, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from keyscan import KEYS

COLUMNS = "kind,battle,day,map,scene,play_s,level,exp,hp,max_hp,junk,wayne,detail".split(",")
MOVABLE = set(KEYS) - {0xC8, 0xC9, 0xD4, 0xD8, 0xD9, 0xDC}   # not the Theater keys or the cut ones

def fields(detail):
    return dict(p.split("=", 1) for p in detail.split() if "=" in p)

def runs(path):
    """Yield (path, run number, {(map, at): (needs, before)}) for each usable run in a play log."""
    run = None
    n = 0
    def finish():
        out = {}
        for place, (index, keys) in run["reach"].items():
            before = {k for k, at in run["lock"].items() if at > index}
            out[place] = (keys, before)
        return out
    for line_no, line in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
        row = line.rstrip("\n").split(",", len(COLUMNS) - 1)
        if len(row) != len(COLUMNS): continue
        r = dict(zip(COLUMNS, row))
        play = int(r["play_s"]) if r["play_s"].isdigit() else None
        if r["kind"] == "rand_newgame":   # the mod's own: the logger's newgame row comes a moment later
            if run and run["ok"]: yield path, n, finish()
            n += 1
            moved = int(fields(r["detail"]).get("keys_moved", "0"))
            if moved: print(f"# {path}:{line_no}: run {n} has moved keys; left out", file=sys.stderr)
            run = {"reach": {}, "lock": {}, "collected": set(), "max_play": 0, "ok": not moved}
            continue
        if run is None or not run["ok"]: continue
        f = fields(r["detail"])
        if r["kind"] == "cardload" and play is not None and play > run["max_play"] + 60:
            print(f"# {path}:{line_no}: run {n} loads a save from another game (play time {play}s > "
                  f"{run['max_play']}s); the run stops here", file=sys.stderr)
            yield path, n, finish()
            run = None
            continue
        if play is not None and r["kind"] != "cardload": run["max_play"] = max(run["max_play"], play)
        if r["kind"] == "rand_layout" and int(f.get("keys_moved", "0")) > 0:
            print(f"# {path}:{line_no}: run {n} has moved keys; left out", file=sys.stderr)
            run["ok"] = False
        elif r["kind"] == "rand_lock":
            run["lock"].setdefault(int(f["item"], 16), line_no)
        elif r["kind"] == "rand_give":
            place = (int(f["map"]), int(f["at"], 16))
            run["reach"].setdefault(place, (line_no, set(run["collected"])))
            item = int(f["item"], 16)
            if item in MOVABLE: run["collected"].add(item)
    if run and run["ok"]: yield path, n, finish()

def main(paths):
    data, seen_in = {}, collections.Counter()
    for path in paths:
        for path_, n, run in runs(path):
            print(f"# {path_} run {n}: {len(run)} spots reached", file=sys.stderr)
            for place, (needs, before) in run.items():
                if place in data: data[place] = (data[place][0] & needs, data[place][1] & before)
                else: data[place] = (set(needs), set(before))
                seen_in[place] += 1
    print("# Keys each spot needs, and keys it may take (keylogic.py). Spot: map+give offset. Runs after #.")
    for (m, at) in sorted(data):
        needs, before = (",".join(f"{k:02X}" for k in sorted(keys)) for keys in data[(m, at)])
        print(f"M{m:03d}+{at:06X} needs={needs} before={before}  # {seen_in[(m, at)]} run(s)")

if __name__ == "__main__":
    if len(sys.argv) < 2: sys.exit(__doc__)
    main(sys.argv[1:])
