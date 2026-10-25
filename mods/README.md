# Mods

Mods for the native Parasite Eve port, built only with `-DPE_MODS=ON`. With it off the game is
the unchanged original: the `PE_HOOK(...)` lines in the game compile to nothing.

```
cmake -S . -B build-mods -G Ninja -DPE_MODS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-mods
```

`PE_MOD_LIST` (CMake cache) picks the mods, by folder name. Default: `pe_qol;pe_rebalance;pe_economy`.

## Layout

```
include/pe_hooks.h     hook ids and context structs: the only file the game includes
mods/
  host/                the mod host, shared by every mod
    pe_mod_host.*        mod registry (start order by requirements), hooks, services, events,
                         pe_mods.log, pe_play.log, pe_toast
    pe_mod_menu.*        rows of the Mods panel and pe_mods.ini
    pe_mod_panel.c       the F1 panel: pages, navigation, input
    pe_pages.*           where PE Enhancements rows live (one layout for all mods)
    pe_game.*            game access: Aya's actor and Combatant, equipped records, base items
    pe_ps1.*             PS1-address access for ported code (fixed-layout objects only)
    ui/                  drawing (pe_ui_raster), the overlay quad (pe_ui_gpu), toasts
  pe_qol/              quality of life: movement (Always run, speeds)
  pe_rebalance/        mod effects, Parasite Energy balance, weapon power (DPS)
  pe_economy/          Junk and loot, Mod Catalog / Dismantle / Rebuild
  pe_hello/            example mod (not built by default)
```

## A mod

A folder with `.c` files, one of which defines its descriptor:

```c
const PeMod pe_mod_<folder> = {"<folder>", "Readable name", "0.1", requires, init, shutdown};
```

`init` registers the mod's rows, hooks and services. Each feature lives in its own file with
one `pe_<feature>_register(mod_id)` function that the mod's main file calls, so a feature can be
moved, disabled or updated without touching the others.

## Talking to the game

- **Hooks** (`pe_hook_add`): one line in the game at a function entry or a fixed moment, with a
  context struct (`include/pe_hooks.h` lists them). Handlers run in priority order; a handler
  that changes a value sees the previous one's result. Add a hook only where a mod needs one,
  and keep it a single line.
- **Game state**: use the game's own globals and structs (`pe_game.h` has the common ones).
  Code ported from the psxrecomp build may use `psx_mod_read/write_*` through `pe_ps1.h`, which
  only maps objects whose layout is the PS1 one (the save block, the resident data pack, the
  card buffer, script variables); other addresses read 0 and are logged once.

## Talking to other mods

- **Services**: `pe_service_publish("junk", 1, &api)` / `pe_service_get("junk", 1)`. A service is
  a versioned struct of functions; a mod that finds none carries on without it.
- **Events**: `pe_event_id("name")` and `pe_event_emit(id, ctx)` for announcements.

Published today: `junk` v1 and `catalog` v1 (pe_economy, see `pe_economy/pe_economy.h`).

## The Mods panel

- PE Enhancements rows go on the pages in `host/pe_pages.h` (Movement, Combat, Economy, Debug);
  tuning and test rows always go under Debug.
- Values the player sets: `toggle`, `choice`, `int` rows (saved in `pe_mods.ini` by page and
  label: renaming a row resets it). Values read from the game: `live_*` rows (not saved).
- Read-only values (Junk, prices, results) are `info` rows, never live rows that refuse changes.
- Every row has a one-line description; actions that cost or destroy something are press-twice.

## Saves

Per-game data goes in the 8 KB card block's free tail (`PE_HOOK_SAVE_WRITE` / `SAVE_READ`),
inside the game's CRC, at the same offsets as the psxrecomp build so saves move between them:

| Offset (from 0x1C90) | Block | Owner |
|---|---|---|
| +0x000 | `PELT` Junk, rare pity, difficulty | pe_economy/pe_loot.c |
| +0x040 | `PECT` Mod Catalog unlocks, Rebuild list | pe_economy/pe_loot_catalog.c |
| +0x1C0 | `PESH` Requisition | (to port) |
| +0x200 | `PKEY` key item pocket | (to port) |
| +0x260 | `SORT` bag sort | (to port) |
