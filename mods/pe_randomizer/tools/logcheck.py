"""Check a storywalk --states file against play logs: every room change a player made (pe_play_log.csv
room rows: from -> map) must be an edge of the walk. Prints the missing ones.
Usage: python -I logcheck.py <walk-states.txt> <pe_play_log.csv> [...]"""
import sys


def main():
    states = {}
    edges = set()
    rows = [l.rstrip("\n").split("\t") for l in open(sys.argv[1])]
    for f in rows: states[int(f[0])] = int(f[1][1:])
    for f in rows:
        a = int(f[1][1:])
        for x in f[5].split():
            edges.add((a, states[int(x.split("[")[0])]))
    seen = {}
    for path in sys.argv[2:]:
        for line in open(path):
            c = line.rstrip("\n").split(",")
            if c[0] != "room": continue
            frm = [x for x in c[12].split() if x.startswith("from=")]
            if not frm: continue
            a = int(frm[0][5:].split("/")[0]); b = int(c[3])
            if a == b or a == 0: continue
            seen.setdefault((a, b), set()).add(c[2])
    miss = [(k, v) for k, v in sorted(seen.items()) if k not in edges]
    print(f"{len(seen)} room changes in the logs, {len(seen) - len(miss)} in the walk, {len(miss)} missing")
    for (a, b), days in miss: print(f"missing\tM{a:03d} -> M{b:03d}\tday {','.join(sorted(days))}")


if __name__ == "__main__":
    main()
