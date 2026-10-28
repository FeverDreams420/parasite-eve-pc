"""Spot table for the pe_randomizer mod, read from a USA disc:
python -I spottable.py <disc.bin> [needs.txt] > pe_rand_spots.h
needs.txt (from keylogic.py) gives the keys each spot needs and the keys it may take; spots it
doesn't list stay unknown.

A spot is one place that hands out an item; a site is one script word the mod rewrites for it
(a spot in several room versions, or with a roll of two items, has several sites). Every site
carries the 16 bytes it expects (step header, mode word, two operands), so the mod only patches
a room whose script is exactly what was scanned here.

  chest   item set at room start, given through an actor slot, with a left-behind-item variable
          (full bag: Aya can swap, the chest keeps her item). Rolls of two count as one spot.
  pickup  item set just before the give (no left-behind variable): keys on the floor.
  gift    GiveItem with the item as an immediate, plus its "You got X" popup when it has one.

Left out (they stay vanilla): Wayne (M048, M052), the debug room (M360), the Chrysler Building,
the shared museum and cruiser routines, ?? lines of chests.py, the Theater keys (their doors
check a story flag), and chests shared by rooms of different days (one chest or a reused
variable: until that's known, moving either could lose an item)."""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dis as D
from keyscan import KEYS

# Save label per map (docs/ROOMS.md, "Map numbers by label"): the area of a spot.
LABELS = [(1, 25, 1), (26, 34, 2), (35, 40, 0), (41, 56, 3), (57, 89, 4), (90, 97, 5), (98, 124, 3),
          (125, 160, 6), (161, 165, 7), (166, 181, 8), (182, 190, 9), (191, 259, 10), (260, 270, 11),
          (271, 271, 12), (272, 273, 13), (274, 287, 14), (319, 332, 6), (333, 333, 11), (334, 334, 2),
          (336, 338, 3), (339, 339, 6), (340, 346, 10), (348, 348, 2), (349, 349, 14), (356, 357, 3),
          (358, 358, 4), (359, 359, 1), (364, 364, 10), (367, 368, 1), (369, 370, 3), (371, 371, 10),
          (372, 373, 1), (374, 375, 4), (376, 376, 3), (377, 378, 1), (379, 379, 10), (380, 381, 6),
          (425, 430, 14), (431, 431, 1), (432, 432, 13), (433, 433, 8)]
AREA_NAMES = ["?", "Theater", "Sewer", "NYPD", "Central Park", "Soho", "Hospital", "Chinatown", "Sewer2",
              "Subway", "Museum", "Warehouse", "Carrier", "Liberty", "Cruiser"]
# Story day by area for maps 288 and up (319-381 repeat earlier places); NYPD is visited on two days.
AREA_DAY = {1: 1, 2: 1, 4: 2, 5: 3, 6: 3, 7: 4, 8: 4, 9: 4, 10: 5, 11: 5, 12: 6, 13: 6, 14: 6}
SKIP_MAPS = {48, 52, 360} | set(range(288, 319)) | set(range(382, 425)) | set(range(434, 438))
GIFT_MAPS = {1, 99, 100, 102, 111, 116, 117, 118, 154}   # character gifts checked by hand (dis.py)
FIXED_KEYS = {0xC8, 0xC9}                                  # Theater: the doors check a story flag
CUT_KEYS = {0xD4, 0xD8, 0xD9, 0xDC}

def area(m): return next((a for lo, hi, a in LABELS if lo <= m <= hi), 0)
def day(m):
    if m <= 34: return 1
    if m <= 89: return 2
    if m <= 160: return 3
    if m <= 190: return 4
    if m <= 270: return 5
    if m <= 287: return 6
    return AREA_DAY.get(area(m), 0)

def word(b, i): return struct.unpack_from("<I", b, i)[0]
def is_step(b, i, op, argc): h = word(b, i); return (h & 0x1FFF) == op and ((h >> 13) & 15) == argc
def site(b, i, operand): return (i, b[i:i + 16], operand)   # patch the operand word (0 or 1) at i + 8 + 4 * operand

def chest_feeds(b, g):
    """(item sites, left-behind variables) feeding the slot given at g: chests.py's walk, keeping offsets."""
    src = word(b, g + 8)
    lo = max(0, g - 0x1400)
    lo = next((i + 4 for i in range(g - 4, lo - 1, -4) if is_step(b, i, 167, 2)), lo)
    feeds = {(1, src): "A"}; items = {}
    for _ in range(3):
        for i in range(lo, g, 4):
            hh, tail = struct.unpack_from("<II", b, i)
            if (hh & 0x1FFF) != 10 or ((hh >> 13) & 15) != 2 or tail != 0 or (hh >> 23): continue
            m0, m1 = (hh >> 17) & 7, (hh >> 20) & 7
            d0, d1 = struct.unpack_from("<II", b, i + 8)
            via = feeds.get((m0, d0))
            if via is None: continue
            if m1 in (1, 2, 4): feeds.setdefault((m1, d1), "V" if m1 == 2 else via)
            elif m1 == 0 and d1 < 0x200 and via == "A": items[i] = d1
    return items, sorted(d for (m, d) in feeds if m == 2)

def scan(path):
    chests, gifts = [], []
    for m in range(1, D.ROOMS + 1):
        if m in SKIP_MAPS: continue
        b = D.room(path, m)
        for g in range(0, len(b) - 24, 4):
            if not is_step(b, g, 167, 2): continue
            mode = (word(b, g) >> 17) & 7
            if mode == 1:
                items, vars_ = chest_feeds(b, g)
                vals = sorted(set(items.values()))
                if not 1 <= len(vals) <= 2 or 0 in vals: continue
                if vars_: kind = "CHEST"
                elif len(items) == 1 and g - next(iter(items)) <= 0x40: kind = "PICKUP"
                else: continue
                last = max(items)   # a roll: the value set last in script order
                chests.append(dict(map=m, give=g, kind=kind, vars=vars_, vanilla=items[last],
                                   sites=[site(b, i, 1) for i in sorted(items)]))
            elif mode == 0:
                v = word(b, g + 8)
                if v in FIXED_KEYS or v in CUT_KEYS: continue
                if not (v in KEYS or m in GIFT_MAPS): continue
                sites = [site(b, g, 0)]
                pop = next((p for p in range(g + 16, min(len(b) - 16, g + 0x100), 4)
                            if word(b, p) == (232 | 1 << 13) and word(b, p + 8) == v), None)
                if pop is not None: sites.append(site(b, pop, 0))
                gifts.append(dict(map=m, give=g, kind="GIFT", vars=[], vanilla=v, sites=sites))
    return chests, gifts

def spots(chests, gifts):
    out = []
    # chests sharing a left-behind variable are one chest in several room versions
    parent = list(range(len(chests)))
    def find(x):
        while parent[x] != x: parent[x] = parent[parent[x]]; x = parent[x]
        return x
    by_var = {}
    for i, c in enumerate(chests):
        for v in c["vars"]:
            if v in by_var: parent[find(i)] = find(by_var[v])
            else: by_var[v] = i
    groups = {}
    for i, c in enumerate(chests): groups.setdefault(find(i), []).append(c)
    for members in groups.values():
        days = {day(c["map"]) for c in members}
        if len(members) > 1 and (len(days) != 1 or 0 in days or len({c["vanilla"] for c in members}) != 1):
            continue   # different days, unknown day or different items: leave them vanilla
        out.append(members)
    # gifts: one spot per key (it may be given in several rooms or branches), else per room and item
    by_gift = {}
    for c in gifts:
        by_gift.setdefault(c["vanilla"] if c["vanilla"] in KEYS else (c["map"], c["vanilla"]), []).append(c)
    out += by_gift.values()
    return sorted(out, key=lambda ms: (ms[0]["map"], ms[0]["give"]))

def read_needs(path):
    """{(map, give): (needs, before)}, sets of key items, from keylogic.py output."""
    out = {}
    for line in open(path):
        line = line.split("#")[0].strip()
        if not line: continue
        place, *parts = line.split()
        m, give = place[1:].split("+")
        f = dict(x.split("=", 1) for x in parts)
        sets = [{int(x, 16) for x in f.get(name, "").split(",") if x} for name in ("needs", "before")]
        if not (sets[0] | sets[1]) <= set(KEYS): sys.exit(f"{path}: not a key item in {line!r}")
        out[(int(m), int(give, 16))] = tuple(sets)
    return out

def emit(spot_list, disc, needs=None):
    sites, gives = [], []
    key_order = [ms[0]["vanilla"] for ms in spot_list if ms[0]["vanilla"] in KEYS]
    print("/* Generated by mods/pe_randomizer/tools/spottable.py from", os.path.basename(disc).replace("*/", ""), "- do not edit.")
    print(" * Places only: map numbers, script offsets, the bytes expected there and vanilla item ids. */")
    print("#pragma once")
    print('#include "pe_randomizer.h"\n')
    print(f"#define PE_RAND_SPOTS {len(spot_list)}")
    print("static const PeRandSpot g_pe_rand_spots[PE_RAND_SPOTS] = {")
    for n, members in enumerate(spot_list):
        c = members[0]; m = c["map"]; v = c["vanilla"]
        flags = ["PE_RAND_KEY"] if v in KEYS else []
        known = [needs[(x["map"], x["give"])] for x in members if needs and (x["map"], x["give"]) in needs]
        mask, before = "PE_RAND_NEEDS_UNKNOWN", "0"
        if known:   # any version a player reached with these keys will do: the one needing fewest
            need, bef = min(known, key=lambda nb: len(nb[0]))
            bits = lambda keys: f"0x{sum(1 << key_order.index(k) for k in keys if k in key_order):08X}"   # Theater keys are fixed
            mask, before = bits(need), bits(bef)
        rooms = " ".join(f"M{x['map']:03d}+{x['give']:06X}" for x in members)
        print(f"    {{ 0x{v:02X}, PE_RAND_{c['kind']}, {day(m)}, {area(m)}, {' | '.join(flags) or 0}, {mask}, {before} }}, "
              f"/* {n}: {rooms} {AREA_NAMES[area(m)]}{' ' + KEYS[v] if v in KEYS else ''} */")
        for x in members:
            for at, raw, operand in x["sites"]: sites.append((x["map"], at, raw, operand, n))
            gives.append((x["map"], x["give"], n))
    print("};\n")
    sites.sort()
    print(f"#define PE_RAND_SITES {len(sites)}")
    print("/* Sorted by map, then offset. */")
    print("static const PeRandSite g_pe_rand_sites[PE_RAND_SITES] = {")
    for m, at, raw, operand, n in sites:
        w = struct.unpack("<4I", raw)
        print(f"    {{ {m}, {n}, 0x{at:06X}, {operand}, {{ 0x{w[0]:08X}, 0x{w[1]:08X}, 0x{w[2]:08X}, 0x{w[3]:08X} }} }},")
    print("};\n")
    gives.sort()
    print(f"#define PE_RAND_GIVES {len(gives)}")
    print("/* Each spot's GiveItem steps (map, offset), sorted: the mod logs which spot gives, for keylogic.py. */")
    print("static const PeRandGive g_pe_rand_gives[PE_RAND_GIVES] = {")
    for m, at, n in gives: print(f"    {{ {m}, {n}, 0x{at:06X} }},")
    print("};")

if __name__ == "__main__":
    if len(sys.argv) not in (2, 3): sys.exit(__doc__)
    sys.stdout.reconfigure(newline="\n")
    c, g = scan(sys.argv[1])
    emit(spots(c, g), sys.argv[1], read_needs(sys.argv[2]) if len(sys.argv) == 3 else None)
