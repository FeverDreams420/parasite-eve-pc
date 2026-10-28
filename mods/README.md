# Mods

Mods for the native Parasite Eve port, built only with `-DPE_MODS=ON`. With it off the game is
the unchanged original: the `PE_HOOK(...)` lines in the game compile to nothing.

```
cmake -S . -B build-mods -G Ninja -DPE_MODS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-mods
```

`PE_MOD_LIST` (CMake cache) picks the mods, by folder name. Default:
`pe_qol` (add `pe_hello` for the example).

Two switches, so a modded build is safe to hand out:
- **Build**: `PE_MODS` off (the default) leaves no mod code in the game at all.
- **Runtime**: F1 > Manage mods > **All mods**, **off by default**. Until a player turns it on and
  restarts, no mod starts and nothing but the F1 panel touches the game. Below it, one toggle per
  mod; a mod turned off doesn't start, nor do the mods that need it. Changes apply at the next start.

The upstream test suite (`-DBUILD_TESTING=ON`) gives the same results with `PE_MODS` on and off.

## Layout

```
include/pe_hooks.h     hook ids and context structs: the only file the game includes
mods/
  host/                the mod host, shared by every mod
    pe_mod_host.*        mod registry (start order by requirements, Manage mods), hooks, services,
                         events, pe_mods.log, pe_play.log (each row also the "play.log" event), pe_toast
    pe_mod_menu.*        rows of the Mods panel and pe_mods.ini
    pe_mod_panel.c       the F1 panel: the list of mods, pages, navigation, input
    pe_pages.*           where PE Enhancements rows live (one layout for the whole suite)
    pe_game.*            game access and the game facts every mod shares (see below)
    ui/                  drawing (pe_ui_raster), the overlay quad (pe_ui_gpu), toasts
  pe_qol/              quality of life: movement, encounters, key item pocket, bag sort, Key Items menu
  pe_hello/            example mod (not built by default)
```

## A mod

A folder with `.c` files, one of which defines its descriptor:

```c
const PeMod pe_mod_<folder> = {"<folder>", "Readable name", "0.1", requires, init, shutdown};
```

Versions: the mod API is `PE_MOD_API_VERSION` (pe_mod_host.h, now 1), raised only when a change
would break existing mods; the PE Enhancements suite is `PE_ENHANCEMENTS_VERSION` (pe_pages.h,
now 0.9.0), one version for all of its mods.

`init` registers the mod's rows, hooks and services. Each feature lives in its own file with
one `pe_<feature>_register(mod_id)` function that the mod's main file calls, so a feature can be
moved, disabled or updated without touching the others. Add the folder name to `PE_MOD_LIST`;
`pe_hello` is the smallest complete example (rows, a hook, a service, an event, its own top page).

## Talking to the game

- **Hooks** (`pe_hook_add`): one line in the game at a function entry or a fixed moment, with a
  context struct (`include/pe_hooks.h` lists them). Handlers run in priority order; a handler
  that changes a value sees the previous one's result. Add a hook only where a mod needs one,
  and keep it a single line.
- **Game state**: use the game's own globals and structs (`pe_game.h` has the common ones),
  never PS1 addresses: native structs hold host pointers and their own layouts.
- **Shared game facts live once, in the host** (`pe_game.h`): `pe_free_field()` (no battle, menu,
  transition or event), `pe_save_loaded()` (also mid room load, when Aya's actor is rebuilt),
  `pe_room_number()`, `pe_wayne_count()` / `pe_set_wayne_count()`. Never copy one of these into a mod.

## Talking to other mods

- **Services**: `pe_service_publish("junk", 1, &api)` / `pe_service_get("junk", 1)`. A service is
  a versioned struct of functions; a mod that finds none carries on without it.
- **Events**: `pe_event_id("name")` and `pe_event_emit(id, ctx)` for announcements.

## The Mods panel

- F1 always opens on the list of mods: each mod's top page (`pe_menu_begin("My Mod")`, described
  and ordered with `pe_menu_top_page`), then the host's Manage mods. The PE Enhancements mods share
  one top page, laid out by topic.
- PE Enhancements rows go on the pages in `host/pe_pages.h` (Movement, Combat, Economy, Inventory,
  Collection, Debug); tuning and test rows always go under Debug.
- Values the player sets: `toggle`, `choice`, `int` rows (saved in `pe_mods.ini` by page and
  label: renaming a row resets it). Values read from the game: `live_*` rows (not saved).
- Read-only values (Junk, prices, results) are `info` rows, never live rows that refuse changes.
- Every row has a one-line description; actions that cost or destroy something are press-twice.

Movement speed uses the local `PLAYER_MOVE` hook in the original input movement
handlers. It never writes the actor's `moveFactor`: cutscenes and battle-entry
scripts own that value, including values that match a mod speed option.

## Optional encounter controls

No random battles (Combat) reads the game's own saved switch. Turning it off
clears only that bit; loading another save or starting a new game clears the
session override. A save made with the switch on retains it.

Each feature has its own file and registration function in pe_qol.

## Battle frequency

Combat / Battle frequency ports the recomp's Normal, More (2x), Lots (4x),
and Every check choices. Normal is the default. It changes the random-roll
range only for the encounter script sequence; unrelated RNG is unchanged.
Every check still respects story gates, room encounter rules and rooms with
no encounters. No random battles overrides it. This increases fight frequency,
not the number of enemies in a fight.

pe_qol/pe_battle_frequency.c owns matching and settings. A single SCRIPT_RANDOM
hook supplies local RNG bounds; loaded scripts and save data are not patched.

## Tests

`tests/run_pe_*_test.ps1` (Windows, after configuring build-mods) build and run each mod's checks
against the production files, with the game and host stubbed where they don't matter.

