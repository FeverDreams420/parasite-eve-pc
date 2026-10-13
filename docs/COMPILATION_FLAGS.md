# Compilation flags of the game code

Every C file is compiled by `tools/scripts/cc.sh`: the stock PSX GCC 2.7.2
`cc1`, then stock MASPSX, then GNU as. The defaults are

```
cc1     -w -O2 -G0 -funsigned-char -mips1 -mcpu=3000
maspsx  --aspsx-version=2.70 --expand-div, assembler -G0
```

`--expand-div` and ASPSX 2.70 became the defaults on 2026-10-12 (they were
2.56 without division checks before): every game unit, overlay unit and
PsyQ unit compiles byte-identically under them except the two exceptions
below, so the 55 marker parts (in 52 units) that spelled them out are gone.

A file changes them with marker comments. This page covers `src/main`
without `src/main/psyq` and `src/overlays`; `tools/reorg/flag_audit.py`
regenerates the tables (`--markdown`), and
`PE_CC_PRINT_FLAGS=1 tools/scripts/cc.sh <file.c> -` prints what cc.sh
resolves for one file. Evidence numbers are **lev**, the Levenshtein
distance over instruction words between the unit's object and the object
compiled without the marker part, measured with the marker removed in place
(same file name, same line numbers).

## How cc.sh reads the markers

cc.sh greps the whole `.c` file (not the headers or `.inc` files it
includes):

- `CC1_FLAGS:` the last `-G<n>` on the line replaces cc1's `-G0`; `-O1`/`-O3`
  replace `-O2`; every other option is matched against a fixed list
  (`-g3`, `-fno-schedule-insns`, `-fno-schedule-insns2`,
  `-fno-strength-reduce`, `-fno-cse-skip-blocks`,
  `-fno-expensive-optimizations`, `-fno-force-mem`, `-ffixed-<reg>`, ...).
  An option that is not on the list is silently ignored.
- `MASPSX_FLAGS:` the last `-G<n>` becomes the assembler `-G`;
  `--dont-expand-li` and `--use-comm-section` are passed to MASPSX;
  `--no-expand-div` drops the default `--expand-div` (cc.sh's own option,
  MASPSX has no negative form).
- `MASPSX_FORCE_G0: 1` keeps MASPSX in its stock forced-G0 mode when the
  assembler `-G` is not 0; without it cc.sh passes `--dont-force-G0`. With
  assembler `-G0` the marker does nothing.
- `ASPSX_VERSION:` the ASPSX version MASPSX emulates (default 2.70).
- `GCC_VERSION: 2.8.1` selects the stock GCC 2.8.1 `cc1`.
- `ASSEMBLER: GNU` assembles cc1's output with plain GNU as (no MASPSX);
  `GAS_VERSION:` selects gas 2.7 / 2.8.1 (only PsyQ units use it).

## The marker block

Every game unit declares its settings in one block at the very top of the
file, one marker per line in a fixed order, then a blank line:

```c
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -G8 -g3 -fno-schedule-insns */
/* ASSEMBLER: GNU */
/* MASPSX_FLAGS: -G8 --use-comm-section --no-expand-div */
/* MASPSX_FORCE_G0: 1 */
/* ASPSX_VERSION: 2.56 */
```

`-G<n>` comes first, the other options in the order of
`tools/scripts/check_flag_markers.py`; options cc.sh does not read, written
defaults and repeated markers are errors, and marker text may appear nowhere
else in the file. `make source-policy-check` runs the check;
`check_flag_markers.py --fix` rewrites a file into the canonical form. The
block only moves and respells markers: it was applied to 62 units with
every binary byte-identical, including the three `-g3` units (their code
lines kept their line numbers).

## What each flag does and why a unit needs it

**cc1 `-G<n>`.** cc1 treats data objects of at most `n` bytes as small data:
it assumes they are reachable in one instruction and does not keep their
address in a register. Retail reaches some words through `$gp`; the cc1
threshold mostly changes register allocation and address reuse (a `-G8`
unit folds `lui/lw ... lui/sw` where a `-G0` unit keeps `&word` in a
register, the REG case of `docs/REORGANIZATION_G_FLAGS.md`).

**maspsx `-G<n>`, `--use-comm-section`, `MASPSX_FORCE_G0`.** The assembler
threshold decides which loads and stores become `$gp`-relative. In forced-G0
mode MASPSX emits `%gp_rel` only for words this file defines as tentative
(COMMON) definitions, and every other extern stays absolute; without
FORCE_G0 GNU as also turns any small extern it knows into `$gp`.
`--use-comm-section` makes MASPSX emit the COMMON definitions it needs to
know their size. The canonical small-data sets are

- **K8:** cc1 `-G8`, maspsx `-G8 --use-comm-section`, `MASPSX_FORCE_G0`,
  plus one COMMON definition per word retail reaches through `$gp`;
- **K0:** the same with cc1 left at `-G0`, for units whose code needs the
  cc1 `-G0` address reuse.

**maspsx `--expand-div` (default) and `--no-expand-div`.** MASPSX expands
`div`/`divu` into the divide plus the divide-by-zero and overflow `break`
checks, as ASPSX does; without it the divide is emitted bare. It is
byte-identical for every unit that does not divide by a variable, so it is
the default. The one exception is `math/fixed_div.c`
(`--no-expand-div`, lev 9 without): its retail divide has no checks (its
neighbours `math/fixed_mul.c` and `math/fixed_round.c` are original assembler).

**maspsx `--dont-expand-li`.** Only acts when the emulated ASPSX is older
than 2.50; with the 2.56 default it does nothing.

**`ASPSX_VERSION` (default 2.70).** MASPSX changes behaviour at 2.60
(`sltu` with a negative immediate no longer goes through `$at`), 2.70 (a
COMMON word may be reached through `$gp` with an offset, `sw v1,4(gp)`) and
2.80 (`la` through `$gp`). Compiling every game unit with 2.60 is identical
to 2.56, so the `sltu` change never matters, and 2.77 configures MASPSX
exactly like 2.70. Every unit except `battle/encounter.c`
compiles byte-identically with 2.70, so it is the default.
`battle/encounter.c` keeps `ASPSX_VERSION: 2.56` (lev 2 without, in
`Battle_SaveAyaState`) because it copies the 8-byte COMMON
`g_BattleEquipStateBlock` and retail stores its second word absolutely.

**`-g3`.** Debug line notes change GCC 2.7.2's scheduling and register
allocation. Three battle units only match with them (lev 1032 to 4292
without); this is the one flag that probably reflects how the original was
built (a debug build of those files).

**`-fno-schedule-insns`.** Disables the first (pre-allocation) scheduling
pass. Retail's order in a few small functions is the unscheduled order.

**`-fno-strength-reduce`.** Disables loop strength reduction: retail keeps
the index multiply / address computation inside the loop.

**`-fno-cse-skip-blocks`, `-fno-expensive-optimizations`, `-fno-force-mem`.**
Narrow CSE and combine options; each is needed by one or two functions whose
retail code keeps a redundant load or a memory operand in place.

**`-ffixed-<reg>`.** Reserves registers for the whole unit. Only the
`scene_e20_cruiser` hover orb uses it (8 registers), as matching debt.

**`GCC_VERSION: 2.8.1`.** No game unit uses it since 2026-10-13 (only
PsyQ units in `src/main/psyq` do). `DecDCTReset` in the `mdec_diag`
overlay used to: its return delay slot is filled, which GCC 2.7.2 with
MASPSX leaves empty, but plain GNU as fills it too, so it now compiles with
GCC 2.7.2 and GNU as like the rest of its LIBPRESS object
(`mdec_diag/decdct.c`).

**`ASSEMBLER: GNU`.** The unit is evidenced original assembler
(`GAME_ASM_FUNCTION`, PsyQ assembler objects). `make source-policy-check`
requires the marker on those units even where MASPSX would produce the same
bytes.

## Markers removed as redundant

Each of these marker parts compiled to a byte-identical object without it,
and the linked binaries stayed identical:

| unit | removed | why it did nothing |
|---|---|---|
| `draw/pipeline.c`, `script/sfx_queue.c` | cc1 `-G0` | the default, written out |
| `fx_common/effects.c` | cc1 `-fno-force-addr` | not an option cc.sh reads |
| `menu_memcard/list_screen.c` | second `-fno-cse-skip-blocks` line | left over from a merge |
| `menu/save_screens.c`, `menu/skills.c`, `script/node_pool.c`, `fx_common/model_buffers.c` | `--dont-expand-li` | no effect at ASPSX 2.56 |
| `battle/attack.c`, `battle/encounter.c`, `room_lib/seeker_spark.c`, `room_lib/drop_field.c` | `--expand-div` | no variable division left in the unit |
| `menu/item_use_panel.c` | `--use-comm-section` | the unit defines no COMMON words |
| `scene_e18_cruiser/e18_emitters.c` | cc1 `-fno-cse-skip-blocks` | the function matches without it |
| `battle/attack.c` | `ASPSX_VERSION` 2.77 written as 2.70 | MASPSX configures both identically |
| 52 units in `src/main` and `src/overlays` | `--expand-div` (49), `ASPSX_VERSION: 2.70` (6) | the defaults since 2026-10-12 |

`MASPSX_FORCE_G0` is also byte-identical without it in
`battle/commands.c`, `menu/equip_select.c`,
`menu/status_panels.c`, `draw/pipeline.c` and
`script/system_opcodes.c`; it stays, because it is part of the canonical
K8/K0 sets those units share with their neighbours. `ASSEMBLER: GNU` on
`math/fixed_mul.c` and `math/fixed_round.c` also stays: source policy
requires it on evidenced assembler.

## Flags replaced by C shapes, and the merges they unblocked

A unit that needed a cc1 option its neighbours do not tolerate was
rewritten until it matched under its neighbours' settings (lev is the only
score; no pins, barriers, `volatile`, `goto` or assembler were added), and
then merged with them:

- `Aya_UnlockParasiteSpell` (`-fno-schedule-insns`, lev 7 without): the
  plain `g_AyaSaveState.parasite_spell_flags |= 1 << spell;` compiles to
  retail's order under the default schedule; the pointer, mask and value
  temporaries were what made the scheduler hoist the `li`. It joined the
  command stack, ammo commit and weapon list units in
  `battle/commands.c` (K8; the ammo commit is the same code
  under K8, and `main/rodata_0019A4` was the 8-alignment pad before the
  weapon-list constants).
- `fx_common` camera (2026-10-13): under K8 `func_8018F05C` scored lev 16
  because cc1 `-G8` treats the 8-byte angle vector `g_FxCommonCameraAngles` as small
  data and stops keeping its address in a register. Declared as an
  incomplete array (`extern GteShortVector g_FxCommonCameraAngles[];`, the
  `RoomFloorY` recipe) it keeps the register under both settings, so
  `camera_nodes.c` and the two `-G8` render ops are one unit,
  `fx_common/camera.c`.

Merges that only needed a redundant marker gone:

- `mdec_diag` (2026-10-13): `DecDCTReset` (GCC 2.8.1), `DecDCTGetEnv` and
  the DecDCT wrappers (default) and `DecDCTPutEnv`, `MdecDiag_Reset`,
  `MdecDiag_SendInput`, `MdecDiag_ReceiveOutput` (`ASSEMBLER: GNU`) all
  compile to the same instructions with GCC 2.7.2 and plain GNU as, so the
  seven units are one `ASSEMBLER: GNU` unit, `mdec_diag/decdct.c`, and no
  game unit uses `GCC_VERSION: 2.8.1` any more.
- `scene_e18_cruiser`: `SceneE18_ExpandingRingCallback` lost its
  `-fno-cse-skip-blocks`, so the model emitters, the ring callback and the
  ring emitters are one unit, `e18_emitters.c` (`--expand-div` is neutral
  for the first two).
- `fx_common`: `FxCommon_UpdateResource` (whose `-fno-force-addr` cc.sh
  never read) joins the effect methods and the motion-delta setup in
  `effects.c`, and `FxCommon_DrawPolyResource` (whose `--dont-expand-li`
  did nothing) joins `model_buffers.c`.

Small-data sets brought to the canonical K8/K0 form (same code; only the
order of symbol-table entries can change, and the link is identical):

- K8 with COMMON definitions of the words retail reaches through `$gp`
  (generated from the old objects by `reorg_merge.py --gp-commons`):
  `cdrom/track_read.c`, `draw/wipe.c`,
  `draw/color_quad.c` (keeps `-fno-schedule-insns`),
  `draw/frame.c`, `entity/frame_update.c`,
  `menu/item_use_panel.c`, `menu/inventory.c`,
  `menu/item_actions.c`, `menu/lists.c`,
  `menu/save_slot.c` (was assembler `-G4`),
  `menu/widget.c`, the timer update in
  `entity/combat.c` (was `-g3 -G1` without FORCE_G0); these units used to let GNU as pick the
  `$gp` words itself (no FORCE_G0).
- K8 by spelling only: `menu/main_update.c` (`-G1` to `-G8`),
  `entity/frame_update.c` (assembler `-G4` to `-G8`),
  `entity/pool.c` (cc1 `-G8`, which is the same code), and the
  two overlay units that had cc1 `-G8` alone, `fx_common/render_op.c` (now
  part of `fx_common/camera.c`) and
  `room_m350_cruiser/RoomEffect_ConfiguredPulseCallback.c` (no COMMON
  words, so FORCE_G0 keeps every access absolute, as before).
- K0: `script/sfx_queue.c` gains `MASPSX_FORCE_G0`.
- Not converted: `render/wireframe.c` (lev 52 under K8: its
  `g_WireScratch` words need GNU as's own `$gp` choice).

Merges unblocked by the canonical sets:

- `entity`: `Scene_LoadMap` (cc1 `-G8`, assembler `-G8` without FORCE_G0)
  and `Entity_FrameUpdate` (assembler `-G4`) are both K8 now and share the
  entity pool words through `$gp`; they are one unit,
  `entity/frame_update.c` (lev 0 against both old objects).
  `entity/pool.c` is K8 too and differs only by `-fno-strength-reduce`
  (lev 3 in `Entity_ResetAllPools` without it; `Scene_LoadMap` does not
  match with it).

## What still needs its own flags

Tried and not removed (lev under the neighbours' settings, or without the
flag):

- `fieldfx/angle.c` (`-fno-schedule-insns`, lev 2): the
  scheduler hoists the target mask above the current mask whatever the
  statement order; the function also carries register pins.
- `draw/gpu_regs.c` (`-fno-schedule-insns`, lev 4) and
  `render/font_load.c` (lev 13): the first sits among GNU-as
  primitive-queue units and is pinned like hand assembler; the second is
  already a contorted shape tuned for the unscheduled order.
- `asset/unload_table.c` (`-fno-force-mem`, lev 1): retail adds
  the scaled index to the table base (`addu v1,v1,v0`); only integer
  arithmetic on the pointer reproduces that, which the debt ratchet counts
  as a new pointer/integer cast.
- `entity/pool.c` (`-fno-strength-reduce`, lev 3): loop.c reduces
  the cell address giv in the record-clearing loop.
- `-g3`: `menu/main_update.c`, `entity/combat.c` and
  `battle/phase.c` now share one set, but their
  other neighbours score lev 276 to 2894 under `-g3`. The drop table and hit
  application take `-g3` with the timer update after them (one unit, same
  bytes).
- `battle/encounter.c` (`ASPSX_VERSION: 2.56`) and `math/fixed_div.c`
  (`--no-expand-div`): the only units that differ under the defaults.
- `render/wireframe.c` (cc1 and assembler `-G8` without
  FORCE_G0): lev 52 under K8.
- Overlays: the render ops of `fx_common/camera.c` and
  `RoomEffect_ConfiguredPulseCallback` need cc1 `-G8` (lev 27 and 7
  without). The camera functions before the render ops share their K8 unit;
  `fx_common/effects.c` after them does not take it (lev 143). The
  `room_lib` units keep one unit per method.
- Overlays: `render_op.c` and `RoomEffect_ConfiguredPulseCallback` need
  cc1 `-G8` (lev 27 and 7 without) and their neighbours do not take it
  (lev 16 to 143). In `room_lib` the class methods that rooms link one by one stay one unit each, and `orbit_set.c` takes `-fno-strength-reduce` for its class too.

## Settings in use

Measured on 2026-10-13 with `tools/reorg/flag_audit.py --markdown`. A
setting is what cc.sh runs, so two files whose markers are spelled
differently but resolve to the same options count once. `--expand-div` and
ASPSX 2.70 are defaults and only their exceptions are shown.

| # | effective setting | main | overlays |
|---|---|---|---|
| S1 | `cc1 -G0 ; maspsx -G0` (default) | 33 | 130 |
| S2 | `cc1 -G8 ; maspsx -G8 --use-comm-section FORCE_G0` | 40 | 2 |
| S3 | `cc1 -G0 ; gnu-as -G0` | 4 | 3 |
| S4 | `cc1 -G0 -fno-schedule-insns ; maspsx -G0` | 3 | 2 |
| S5 | `cc1 -G0 -fno-strength-reduce ; maspsx -G0` | 3 | 2 |
| S6 | `cc1 -G0 ; maspsx -G8 --use-comm-section FORCE_G0` | 3 | 0 |
| S7 | `cc1 -G8 -g3 ; maspsx -G8 --use-comm-section FORCE_G0` | 3 | 0 |
| S8 | `cc1 -G0 -fno-cse-skip-blocks ; maspsx -G0` | 0 | 2 |
| S9 | `cc1 -G8 -fno-strength-reduce ; maspsx -G8 --use-comm-section FORCE_G0` | 2 | 0 |
| S10 | `cc1 -G0 -ffixed-10 -ffixed-11 -ffixed-15 -ffixed-21 -ffixed-22 -ffixed-23 -ffixed-24 -ffixed-25 ; maspsx -G0` | 0 | 1 |
| S11 | `cc1 -G0 -fno-expensive-optimizations ; maspsx -G0` | 0 | 1 |
| S12 | `cc1 -G0 -fno-force-mem ; maspsx -G0` | 1 | 0 |
| S13 | `cc1 -G0 -fno-strength-reduce -fno-expensive-optimizations ; maspsx -G0` | 1 | 0 |
| S14 | `cc1 -G0 ; maspsx -G0 --no-expand-div` | 1 | 0 |
| S15 | `cc1 -G8 -fno-cse-skip-blocks ; maspsx -G8 --use-comm-section FORCE_G0` | 1 | 0 |
| S16 | `cc1 -G8 -fno-expensive-optimizations ; maspsx -G8 --use-comm-section FORCE_G0` | 1 | 0 |
| S17 | `cc1 -G8 -fno-schedule-insns ; maspsx -G8 --use-comm-section FORCE_G0` | 1 | 0 |
| S18 | `cc1 -G8 ; maspsx -G8` | 1 | 0 |
| S19 | `cc1 -G8 ; maspsx -G8 --use-comm-section FORCE_G0 aspsx 2.56` | 1 | 0 |

19 distinct settings over 242 files.

#### S2: `cc1 -G8 ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/aya/stats.c`
- `src/main/battle/attack.c`
- `src/main/battle/commands.c`
- `src/main/battle/enemy_movement.c`
- `src/main/battle/player_turn.c`
- `src/main/battle/setup.c`
- `src/main/battle/status_hud.c`
- `src/main/battle/turn.c`
- `src/main/boot/frame_loop.c`
- `src/main/boot/init.c`
- `src/main/cdrom/track_read.c`
- `src/main/draw/frame.c`
- `src/main/draw/textbox.c`
- `src/main/draw/wipe.c`
- `src/main/entity/frame_update.c`
- `src/main/field/actor_collision.c`
- `src/main/item/actions.c`
- `src/main/item/active_list.c`
- `src/main/item/armor_capacity.c`
- `src/main/item/swap.c`
- `src/main/menu/equip_screen.c`
- `src/main/menu/equip_select.c`
- `src/main/menu/input_queue.c`
- `src/main/menu/inventory.c`
- `src/main/menu/item_actions.c`
- `src/main/menu/item_use_panel.c`
- `src/main/menu/lists.c`
- `src/main/menu/main_menu.c`
- `src/main/menu/save_screens.c`
- `src/main/menu/save_slot.c`
- `src/main/menu/skills.c`
- `src/main/menu/status_panels.c`
- `src/main/menu/widget.c`
- `src/main/render/frame_update.c`
- `src/main/render/object.c`
- `src/main/save/game_data.c`
- `src/main/scene/room_load.c`
- `src/main/script/actor_opcodes.c`
- `src/main/script/flow_opcodes.c`
- `src/main/script/node_actions.c`
- `src/overlays/fx_common/camera.c`
- `src/overlays/room_m350_cruiser/RoomEffect_ConfiguredPulseCallback.c`

#### S3: `cc1 -G0 ; gnu-as -G0`

- `src/main/draw/prim_color.c`
- `src/main/math/fixed_mul.c`
- `src/main/math/fixed_round.c`
- `src/main/math/sqrt_approx.c`
- `src/overlays/mdec_diag/decdct.c`
- `src/overlays/mdec_diag/vlc.c`
- `src/overlays/mdec_diag/vlc_c.c`

#### S4: `cc1 -G0 -fno-schedule-insns ; maspsx -G0`

- `src/main/draw/gpu_regs.c`
- `src/main/fieldfx/angle.c`
- `src/main/render/font_load.c`
- `src/overlays/fx_common/motion_endpoints.c`
- `src/overlays/room_m273_liberty/RoomM273_PulseSpriteCallback.c`

#### S5: `cc1 -G0 -fno-strength-reduce ; maspsx -G0`

- `src/main/akao/voice_bank.c`
- `src/main/fieldfx/collision.c`
- `src/main/render/mesh_list.c`
- `src/overlays/room_lib/motion_particles_2.c`
- `src/overlays/room_lib/orbit_set.c`

#### S6: `cc1 -G0 ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/draw/pipeline.c`
- `src/main/script/sfx_queue.c`
- `src/main/script/system_opcodes.c`

#### S7: `cc1 -G8 -g3 ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/battle/phase.c`
- `src/main/entity/combat.c`
- `src/main/menu/main_update.c`

#### S8: `cc1 -G0 -fno-cse-skip-blocks ; maspsx -G0`

- `src/overlays/menu_memcard/page_flip.c`
- `src/overlays/menu_memcard/list_screen.c`

#### S9: `cc1 -G8 -fno-strength-reduce ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/entity/pool.c`
- `src/main/script/node_pool.c`

#### S10: `cc1 -G0 -ffixed-10 -ffixed-11 -ffixed-15 -ffixed-21 -ffixed-22 -ffixed-23 -ffixed-24 -ffixed-25 ; maspsx -G0`

- `src/overlays/scene_e20_cruiser/SceneE20_HoverOrbController.c`

#### S11: `cc1 -G0 -fno-expensive-optimizations ; maspsx -G0`

- `src/overlays/fx_common/effect_init.c`

#### S12: `cc1 -G0 -fno-force-mem ; maspsx -G0`

- `src/main/asset/unload_table.c`

#### S13: `cc1 -G0 -fno-strength-reduce -fno-expensive-optimizations ; maspsx -G0`

- `src/main/render/mesh_entry.c`

#### S14: `cc1 -G0 ; maspsx -G0 --no-expand-div`

- `src/main/math/fixed_div.c`

#### S15: `cc1 -G8 -fno-cse-skip-blocks ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/draw/text.c`

#### S16: `cc1 -G8 -fno-expensive-optimizations ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/menu/item_detail.c`

#### S17: `cc1 -G8 -fno-schedule-insns ; maspsx -G8 --use-comm-section FORCE_G0`

- `src/main/draw/color_quad.c`

#### S18: `cc1 -G8 ; maspsx -G8`

- `src/main/render/wireframe.c`

#### S19: `cc1 -G8 ; maspsx -G8 --use-comm-section FORCE_G0 aspsx 2.56`

- `src/main/battle/encounter.c`

## Evidence per unit

Measured on the final tree. Each marker part of each non-default unit was
removed on its own (and the whole small-data part together) and the unit
recompiled; "identical" means
the object is byte-identical without it. "Same code, symbol table differs"
means only the order of symbol-table entries changed (cc1 `-G8` emits
`.extern` lines for small words). The `--expand-div` and `ASPSX_VERSION 2.70`
entries were measured while those were markers; they are now the defaults,
so the numbers say what the unit scores with `--no-expand-div` or
`ASPSX_VERSION: 2.56`.

| unit | without this marker part (lev, worst functions) |
|---|---|
| `main/akao/sequencer.c` | maspsx `--expand-div`: lev 119 (Akao_StepSampleLoader 15, SeqOp_NoteOnWithPitchSlide 13, SeqOp_SlideVolume 9) |
| `main/akao/voice_bank.c` | cc1 `-fno-strength-reduce`: lev 496 (Akao_StepSequencerVoice 62, Spu_SlideVoicePanMasked 45, Spu_ParameterSlide 45); maspsx `--expand-div`: lev 209 (Spu_SlideVoicePitchMasked 26, Spu_SlideVoicePanMasked 26, Spu_ParameterSlide 26) |
| `main/asset/unload_table.c` | cc1 `-fno-force-mem`: lev 1 (Asset_UnloadTableEntries 1) |
| `main/aya/stats.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 2 (Inv_GetBonusSlotCount 2); maspsx `--use-comm-section`: lev 1 (Inv_GetBonusSlotCount 1); FORCE_G0: lev 18 (Inv_RecalcSlotStats 7, Battle_GetEnemyEscapeFlag 5, Battle_SyncEquipSlots 4); small-data set: lev 2 (Inv_GetBonusSlotCount 2) |
| `main/battle/attack.c` | cc1 `-G8`: lev 182 (Battle_ApplySpellEffect 79, Battle_StepAyaAction 58, Battle_StepCharacterAction 45); maspsx `-G8`: lev 395 (Battle_StepAyaAction 167, Battle_StepCharacterAction 90, Battle_ResolveHitOnTimer 90); maspsx `--use-comm-section`: lev 116 (Battle_StepAyaAction 58, Battle_ResolveHitOnTimer 27, Battle_StepCharacterAction 25); FORCE_G0: lev 569 (Battle_StepAyaAction 151, Battle_ApplySpellEffect 125, Battle_ResolveHitOnTimer 96); ASPSX_VERSION 2.70: lev 63 (Battle_StepAyaAction 63); small-data set: lev 452 (Battle_StepAyaAction 176, Battle_StepCharacterAction 90, Battle_ResolveHitOnTimer 90) |
| `main/battle/commands.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 93 (Inv_SetActiveList 37, BattleCmd_LoadWeaponModifiers 13, BattleCmd_UndoPending 12); maspsx `--use-comm-section`: lev 20 (Inv_SetActiveList 7, BattleCmd_UndoPending 2, BattleCmd_UndoAllPending 2); FORCE_G0: lev 68 (Inv_SetActiveList 26, BattleCmd_GetRemainingAmmo 8, BattleCmd_ChangeWeaponAndSync 8); small-data set: lev 93 (Inv_SetActiveList 37, BattleCmd_LoadWeaponModifiers 13, BattleCmd_UndoPending 12) |
| `main/battle/encounter.c` | ASPSX_VERSION 2.56: lev 2 (Battle_SaveAyaState 2); cc1 `-G8`: lev 12 (Battle_Update 6, Battle_PhaseInitEnemyTurn 6); maspsx `-G8`: lev 670 (Battle_Update 170, Battle_StepPostBattle 89, Battle_StepVictory 85); maspsx `--use-comm-section`: lev 176 (Battle_Update 44, Battle_StepVictory 32, Battle_StepPostBattle 24); FORCE_G0: lev 1295 (Battle_PhaseInitEnemyTurn 289, Battle_PhaseEndTurn 275, Battle_Update 264); small-data set: lev 682 (Battle_Update 176, Battle_StepPostBattle 89, Battle_StepVictory 85) |
| `main/battle/enemy_movement.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: identical; maspsx `--use-comm-section`: identical; maspsx `--expand-div`: lev 70 (Battle_StepEnemyMovement 70); FORCE_G0: lev 169 (Battle_StepEntityAnimState 73, Battle_StepEnemyMovement 71, Battle_UpdateEntityFacing 25); small-data set: same code, symbol table differs |
| `main/battle/phase.c` | cc1 `-G8`: same code, symbol table differs; cc1 `-g3`: lev 1588 (Battle_AdvancePhase 633, Battle_StartEnemyAttackEffect 161, LM196 37); maspsx `-G8`: lev 61 (LM196 3, LM93 2, LM9 2); maspsx `--use-comm-section`: lev 30 (LM93 1, LM9 1, LM77 1); maspsx `--expand-div`: identical; FORCE_G0: lev 99 (LM196 5, LM82 4, LM81 4); small-data set: lev 61 (LM196 3, LM93 2, LM9 2) |
| `main/battle/player_turn.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 51 (Battle_UpdatePlayerTurn 42, Battle_ApplyPlayerHit 9); maspsx `--use-comm-section`: lev 14 (Battle_UpdatePlayerTurn 11, Battle_ApplyPlayerHit 3); FORCE_G0: lev 74 (Battle_UpdatePlayerTurn 44, Battle_ApplyPlayerHit 30); small-data set: lev 51 (Battle_UpdatePlayerTurn 42, Battle_ApplyPlayerHit 9) |
| `main/battle/setup.c` | cc1 `-G8`: lev 90 (Battle_CopyPadStateToRecord 57, Battle_SyncEnemyAttributes 33); maspsx `-G8`: lev 137 (Battle_StepScriptEntry 28, Battle_Init 27, Battle_BuildTargetList 22); maspsx `--use-comm-section`: lev 47 (Battle_Init 13, Battle_StepScriptEntry 8, Battle_BuildTargetList 7); maspsx `--expand-div`: lev 42 (Battle_CheckEvasion 42); FORCE_G0: lev 247 (Battle_CheckEvasion 48, Battle_SyncEnemyAttributes 32, Battle_DispatchEntityEffect 32); small-data set: lev 227 (Battle_CopyPadStateToRecord 57, Battle_SyncEnemyAttributes 33, Battle_StepScriptEntry 28) |
| `main/battle/status_hud.c` | cc1 `-G8`: lev 80 (Battle_DrawSpellName 51, Battle_DrawActiveStatus 29); maspsx `-G8`: lev 208 (Battle_DrawActiveStatus 75, Battle_DrawStatusPanel 57, Battle_DrawEnemyHP 26); maspsx `--use-comm-section`: lev 73 (Battle_DrawActiveStatus 34, Battle_DrawStatusPanel 20, Render_AnimationFrame 6); maspsx `--expand-div`: lev 16 (Battle_DrawActiveStatus 16); FORCE_G0: lev 389 (Battle_DrawActiveStatus 62, Battle_DrawStatusPanel 53, Battle_DrawEnemyHP 53); ASPSX_VERSION 2.70: lev 65 (Battle_DrawActiveStatus 45, Battle_DrawEnemyHP 20); small-data set: lev 255 (Battle_DrawActiveStatus 71, Battle_DrawStatusPanel 57, Battle_DrawSpellName 51) |
| `main/battle/turn.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 625 (Battle_HandleItemMenu 122, Battle_StepEnemyTurn 116, Battle_StepPlayerTurn 86); maspsx `--use-comm-section`: lev 155 (Battle_StepEnemyTurn 41, Battle_HandleItemMenu 30, Battle_AdvanceTurnSlot 27); maspsx `--expand-div`: lev 80 (Battle_StepEnemyTurn 35, Battle_CycleTarget 31, Battle_FillActionQueue 14); FORCE_G0: lev 644 (Battle_HandleItemMenu 108, Battle_UpdateEnemy 94, Battle_StepPlayerTurn 84); ASPSX_VERSION 2.70: lev 6 (Battle_SetupEntityTarget 6); small-data set: lev 625 (Battle_HandleItemMenu 122, Battle_StepEnemyTurn 116, Battle_StepPlayerTurn 86) |
| `main/boot/frame_loop.c` | cc1 `-G8`: lev 13 (Boot_RunFrame 13); maspsx `-G8`: lev 61 (Boot_RunFrame 56, Boot_FlushSceneFast 5); maspsx `--use-comm-section`: lev 17 (Boot_RunFrame 15, Boot_FlushSceneFast 2); FORCE_G0: lev 52 (Boot_RunFrame 52); small-data set: lev 63 (Boot_RunFrame 58, Boot_FlushSceneFast 5) |
| `main/boot/init.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 161 (Field_HandleStateTransition 150, Boot_BuildRenderFlagTable 11); maspsx `--use-comm-section`: lev 55 (Field_HandleStateTransition 50, Boot_BuildRenderFlagTable 5); FORCE_G0: lev 101 (Field_HandleStateTransition 86, Boot_InitSubsystems 9, Boot_BuildRenderFlagTable 4); small-data set: lev 161 (Field_HandleStateTransition 150, Boot_BuildRenderFlagTable 11) |
| `main/cdrom/scene_assets.c` | maspsx `--expand-div`: lev 19 (Akao_Calc3DPan 19) |
| `main/cdrom/track_read.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 206 (CdRom_StepReadState 77, CdRom_ReadAssetSectors 54, CdRom_SeekToTrack 53); maspsx `--use-comm-section`: lev 48 (CdRom_StepReadState 19, CdRom_ReadAssetSectors 12, CdRom_SeekToTrack 11); FORCE_G0: identical; small-data set: lev 206 (CdRom_StepReadState 77, CdRom_ReadAssetSectors 54, CdRom_SeekToTrack 53) |
| `main/draw/color_quad.c` | cc1 `-G8`: identical; cc1 `-fno-schedule-insns`: lev 107 (Draw_AllocColorQuad 107); maspsx `-G8`: lev 70 (Draw_AllocColorQuad 70); maspsx `--use-comm-section`: lev 24 (Draw_AllocColorQuad 24); maspsx `--expand-div`: lev 23 (Draw_AllocColorQuad 23); FORCE_G0: identical; small-data set: lev 70 (Draw_AllocColorQuad 70) |
| `main/draw/frame.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 13 (Draw_RenderFrame 13); maspsx `--use-comm-section`: lev 5 (Draw_RenderFrame 5); FORCE_G0: identical; small-data set: lev 13 (Draw_RenderFrame 13) |
| `main/draw/gpu_regs.c` | cc1 `-fno-schedule-insns`: lev 4 (Draw_InitGpuHwRegs 4) |
| `main/draw/pipeline.c` | maspsx `-G8`: lev 17 (Draw_InitPipeline 17); maspsx `--use-comm-section`: lev 4 (Draw_InitPipeline 4); FORCE_G0: identical; small-data set: lev 17 (Draw_InitPipeline 17) |
| `main/draw/text.c` | cc1 `-G8`: same code, symbol table differs; cc1 `-fno-cse-skip-blocks`: lev 22 (Draw_AllocTexturedQuad 22); maspsx `-G8`: lev 907 (Draw_AllocTexturedRectAlt 211, Draw_PrintTimeValue 81, Draw_PrintCenteredTextInWidth 65); maspsx `--use-comm-section`: lev 343 (Draw_AllocTexturedRectAlt 82, Draw_PrintTimeValue 38, Draw_PrintCenteredTextInWidth 23); maspsx `--expand-div`: lev 73 (Draw_AllocTexturedRect 13, Draw_PrintSignedNumberWidth4 12, Draw_PrintSignedNumberWidth3 12); FORCE_G0: lev 4 (Draw_SetBaseOffsetPosition 2, Draw_GetBaseY 2); small-data set: lev 907 (Draw_AllocTexturedRectAlt 211, Draw_PrintTimeValue 81, Draw_PrintCenteredTextInWidth 65) |
| `main/draw/textbox.c` | cc1 `-G8`: lev 20 (Render_SetupColorTable 20); maspsx `-G8`: lev 259 (Menu_DrawTextboxEntries 186, Render_SetupColorTable 24, Draw_SetLightingParams 14); maspsx `--use-comm-section`: lev 79 (Menu_DrawTextboxEntries 49, Render_SetupColorTable 7, Draw_SetLightingParams 7); FORCE_G0: lev 108 (Menu_DrawTextboxEntries 108); ASPSX_VERSION 2.70: lev 22 (Render_SetupColorTable 16, Menu_SetTextCursorRect 6); small-data set: lev 261 (Menu_DrawTextboxEntries 186, Render_SetupColorTable 26, Draw_SetLightingParams 14) |
| `main/draw/wipe.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 335 (Draw_AllocColorGradient 113, Draw_AllocColorTriGradient 66, Draw_EmitWipeBarRect 40); maspsx `--use-comm-section`: lev 119 (Draw_AllocColorGradient 42, Draw_AllocColorTriGradient 24, Draw_EmitWipeBarRect 16); FORCE_G0: identical; small-data set: lev 335 (Draw_AllocColorGradient 113, Draw_AllocColorTriGradient 66, Draw_EmitWipeBarRect 40) |
| `main/entity/combat.c` (drop table, hit; measured as a `-G8` unit) | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 13 (Entity_ApplyHitAndSetAnim 13); maspsx `--use-comm-section`: lev 3 (Entity_ApplyHitAndSetAnim 3); maspsx `--expand-div`: lev 14 (Entity_ResolveDropTable 14); FORCE_G0: lev 54 (Entity_ResolveDropTable 31, Entity_ApplyHitAndSetAnim 23); small-data set: lev 13 (Entity_ApplyHitAndSetAnim 13) |
| `main/entity/combat.c` (timer update) | cc1 `-G8`: same code, symbol table differs; cc1 `-g3`: lev 1032 (Entity_UpdateTimers 516, LM51 23, LM41 13); maspsx `-G8`: lev 4 (LM59 2, LM58 2); maspsx `--use-comm-section`: lev 2 (LM59 1, LM58 1); maspsx `--expand-div`: lev 9 (LM51 9); FORCE_G0: lev 64 (LM51 5, LM18 4, LM9 2); small-data set: lev 4 (LM59 2, LM58 2) |
| `main/entity/frame_update.c` | cc1 `-G8`: lev 70 (Entity_FrameUpdate 70); maspsx `-G8`: lev 180 (Entity_FrameUpdate 71, Scene_LoadMap 50, Entity_CollectGarbage 35); maspsx `--use-comm-section`: lev 39 (Entity_CollectGarbage 12, Scene_LoadMap 11, Entity_FrameUpdate 11); FORCE_G0: lev 81 (Entity_FrameUpdate 73, Entity_IntegratePositionConditional 5, Entity_DispatchCallbacks 3); small-data set: lev 205 (Entity_FrameUpdate 96, Scene_LoadMap 50, Entity_CollectGarbage 35) |
| `main/entity/pool.c` | cc1 `-G8`: same code, symbol table differs; cc1 `-fno-strength-reduce`: lev 3 (Entity_ResetAllPools 3); maspsx `-G8`: lev 28 (Entity_ResetAllPools 18, Entity_InitFreePool 10); maspsx `--use-comm-section`: lev 11 (Entity_ResetAllPools 7, Entity_InitFreePool 4); FORCE_G0: identical; small-data set: lev 28 (Entity_ResetAllPools 18, Entity_InitFreePool 10) |
| `main/field/actor_collision.c` | cc1 `-G8`: lev 68 (Entity_SlideOnRamp 68); maspsx `-G8`: lev 562 (Geo_ClipToFloorBoundary 165, Entity_UpdateAndRender 100, Geo_ClipToFloorBoundarySub 53); maspsx `--use-comm-section`: lev 150 (Geo_ClipToFloorBoundary 48, Entity_UpdateAndRender 22, Entity_ResetStateGlobals 16); maspsx `--expand-div`: lev 193 (Geo_ClipToFloorBoundary 76, Entity_SlideOnWall 36, Entity_SlideOnRamp 36); FORCE_G0: lev 196 (Geo_ClipToFloorBoundary 71, Entity_UpdateAndRender 69, Entity_SlideOnRamp 28); ASPSX_VERSION 2.70: lev 93 (Geo_ClipToFloorBoundary 61, Entity_SlideOnRamp 32); small-data set: lev 592 (Geo_ClipToFloorBoundary 165, Entity_UpdateAndRender 100, Entity_SlideOnRamp 82) |
| `main/fieldfx/angle.c` | cc1 `-fno-schedule-insns`: lev 2 (FieldFx_TurnToward 2) |
| `main/fieldfx/collision.c` | cc1 `-fno-strength-reduce`: lev 53 (FieldFx_UpdateStripCollision 53) |
| `main/fieldfx/emitters.c` | maspsx `--expand-div`: lev 14 (FieldFx_CosineEffect 14) |
| `main/fieldfx/geometry.c` | maspsx `--expand-div`: lev 143 (FieldFx_DrawRingBand 34, FieldFx_DrawTexturedRibbon 33, FieldFx_DrawStarFan 29) |
| `main/fieldfx/sprite_effects.c` | maspsx `--expand-div`: lev 105 (func_800D5010 32, FieldFx_OrbitingSprite 26, FieldFx_CosineEffectB 24) |
| `main/fieldfx/sprite_render.c` | maspsx `--expand-div`: lev 46 (FieldFx_DrawBillboard 21, FieldFx_BuildRingGeometry 13, FieldFx_DrawShadedRing 6) |
| `main/item/actions.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 932 (Inv_BuildCompatibleWeaponBitset 217, Inv_RebuildSelectableMask 181, Inv_InitSlotDisplay 94); maspsx `--use-comm-section`: lev 242 (Inv_RebuildSelectableMask 50, Inv_BuildCompatibleWeaponBitset 48, Inv_InitSlotDisplay 37); FORCE_G0: lev 25 (Inv_RemoveActiveListItem 11, Inv_CheckFreeSlotCapacity 8, Item_PackParasiteSpells 6); small-data set: lev 932 (Inv_BuildCompatibleWeaponBitset 217, Inv_RebuildSelectableMask 181, Inv_InitSlotDisplay 94) |
| `main/item/active_list.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 580 (Inv_SetupSlotDisplay 96, Inv_PickRandomItem 44, Inv_WriteSlotById 38); maspsx `--use-comm-section`: lev 164 (Inv_SetupSlotDisplay 31, Inv_PickRandomItem 12, Inv_SelectActiveList 10); FORCE_G0: lev 11 (Inv_CompactActiveListSlots 11); small-data set: lev 580 (Inv_SetupSlotDisplay 96, Inv_PickRandomItem 44, Inv_WriteSlotById 38) |
| `main/item/armor_capacity.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 697 (Inv_TransferItemBetweenLists 263, Inv_BuildStorageDisplay 78, Inv_SortInventoryByMode 63); maspsx `--use-comm-section`: lev 187 (Inv_TransferItemBetweenLists 58, Inv_BuildStorageDisplay 28, Inv_SortInventoryByMode 22); FORCE_G0: lev 9 (Inv_GetActiveSlotCount 9); small-data set: lev 697 (Inv_TransferItemBetweenLists 263, Inv_BuildStorageDisplay 78, Inv_SortInventoryByMode 63) |
| `main/item/swap.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 397 (Inv_RebuildWithSlotLimit 91, Inv_TransferItemAlt 47, Inv_TransferItem 46); maspsx `--use-comm-section`: lev 108 (Inv_RebuildWithSlotLimit 18, Inv_TransferItem 17, Inv_TransferItemAlt 13); FORCE_G0: lev 90 (Inv_TransferItem 56, Inv_TransferItemAlt 27, Inv_CanAddActiveListItemToAya 7); small-data set: lev 397 (Inv_RebuildWithSlotLimit 91, Inv_TransferItemAlt 47, Inv_TransferItem 46) |
| `main/math/fixed_div.c` | maspsx `--no-expand-div`: lev 9 (Math_FixedDivide 9) |
| `main/menu/equip_screen.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 185 (Menu_InventoryInputHandler 56, Menu_StepEquipScreen 50, Menu_OpenItemActionSubmenu 44); maspsx `--use-comm-section`: lev 46 (Menu_OpenItemActionSubmenu 13, Menu_StepEquipScreen 11, Menu_OnInventoryItemConfirm 9); FORCE_G0: lev 13 (Menu_OnInventoryItemConfirm 13); small-data set: lev 185 (Menu_InventoryInputHandler 56, Menu_StepEquipScreen 50, Menu_OpenItemActionSubmenu 44) |
| `main/menu/equip_select.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 177 (Menu_CreateAmmoSpendPanel 36, Menu_StepEquipSlotSelect 29, Menu_StepItemGridScroll 24); maspsx `--use-comm-section`: lev 44 (Menu_CreateAmmoSpendPanel 17, Menu_StepEquipSlotSelect 6, Menu_AmmoSpendInputHandler 6); FORCE_G0: identical; small-data set: lev 177 (Menu_CreateAmmoSpendPanel 36, Menu_StepEquipSlotSelect 29, Menu_StepItemGridScroll 24) |
| `main/menu/input_queue.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 223 (MenuInput_EnqueueStatusChanges 70, MenuInput_DispatchQueuedEvents 38, Queue_DequeueByMask 22); maspsx `--use-comm-section`: lev 76 (MenuInput_EnqueueStatusChanges 24, Draw_SelectBuffer 9, MenuInput_DispatchQueuedEvents 7); FORCE_G0: lev 19 (Draw_RemapStatusFlags 15, Draw_SelectBuffer 4); small-data set: lev 223 (MenuInput_EnqueueStatusChanges 70, MenuInput_DispatchQueuedEvents 38, Queue_DequeueByMask 22) |
| `main/menu/inventory.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 228 (Menu_OpenInventoryScreen 39, Menu_StepInventoryCategory 29, Menu_InventoryPageNavHandler 25); maspsx `--use-comm-section`: lev 80 (Menu_OpenInventoryScreen 15, Menu_ResetStateFlags 10, Menu_StepInventoryCategory 6); FORCE_G0: identical; small-data set: lev 228 (Menu_OpenInventoryScreen 39, Menu_StepInventoryCategory 29, Menu_InventoryPageNavHandler 25) |
| `main/menu/item_actions.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 637 (Menu_ItemListInputHandler 157, Menu_StepInventoryRoot 87, Menu_SpendBonusPoints 83); maspsx `--use-comm-section`: lev 182 (Menu_ItemListInputHandler 41, Menu_SpendBonusPoints 27, Menu_StepInventoryRoot 26); FORCE_G0: identical; small-data set: lev 637 (Menu_ItemListInputHandler 157, Menu_StepInventoryRoot 87, Menu_SpendBonusPoints 83) |
| `main/menu/item_detail.c` | cc1 `-G8`: same code, symbol table differs; cc1 `-fno-expensive-optimizations`: lev 34 (Menu_StepItemDetailPanel 34); maspsx `-G8`: lev 37 (Menu_StepItemDetailPanel 15, Menu_ConfirmDialogHandler 12, Menu_OnItemDiscardConfirm 10); maspsx `--use-comm-section`: lev 13 (Menu_StepItemDetailPanel 6, Menu_OnItemDiscardConfirm 3, Menu_DrawItemLabel 2); FORCE_G0: lev 3 (Menu_DrawItemLabel 3); small-data set: lev 37 (Menu_StepItemDetailPanel 15, Menu_ConfirmDialogHandler 12, Menu_OnItemDiscardConfirm 10) |
| `main/menu/item_use_panel.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 22 (Menu_OpenItemUsePanelAtIndex 22); maspsx `--use-comm-section`: lev 4 (Menu_OpenItemUsePanelAtIndex 4); FORCE_G0: identical; small-data set: lev 22 (Menu_OpenItemUsePanelAtIndex 22) |
| `main/menu/lists.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 129 (Menu_ContextPick 23, Menu_DrawItemSlot 13, Menu_SoundTestSelect 11); maspsx `--use-comm-section`: lev 40 (Menu_ContextPick 5, Menu_DrawItemSlot 3, Menu_DrawEquipItemDetailPanel 3); FORCE_G0: identical; small-data set: lev 129 (Menu_ContextPick 23, Menu_DrawItemSlot 13, Menu_SoundTestSelect 11) |
| `main/menu/main_menu.c` | cc1 `-G8`: lev 11 (Aya_SetTotalExp 11); maspsx `-G8`: lev 252 (Menu_DrawBonusPointAnimFrame 72, Aya_SetTotalExp 39, Menu_StepParasiteScreen 37); maspsx `--use-comm-section`: lev 100 (Menu_DrawBonusPointAnimFrame 29, Aya_SetTotalExp 18, Menu_StepParasiteScreen 14); FORCE_G0: lev 9 (Aya_SetTotalExp 9); small-data set: lev 257 (Menu_DrawBonusPointAnimFrame 72, Aya_SetTotalExp 44, Menu_StepParasiteScreen 37) |
| `main/menu/main_update.c` | cc1 `-G8`: same code, symbol table differs; cc1 `-g3`: lev 4292 (Menu_MainUpdate 2146, LM35 26, LM22 22); maspsx `-G8`: lev 10 (LM352 2, LM323 2, LM166 2); maspsx `--use-comm-section`: lev 5 (LM352 1, LM323 1, LM166 1); maspsx `--expand-div`: lev 18 (LM424 9, LM22 9); FORCE_G0: lev 372 (LM422 4, LM421 4, LM40 4); small-data set: lev 10 (LM352 2, LM323 2, LM166 2) |
| `main/menu/save_screens.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 310 (Menu_DrawContextHelpText 107, Menu_StepNameEntryAlt 43, Menu_StepSaveSelect 41); maspsx `--use-comm-section`: lev 67 (Menu_DrawContextHelpText 18, Menu_StepSaveSelect 13, Menu_StepItemGrid2 6); FORCE_G0: lev 59 (Menu_StepNameEntryAlt 46, Menu_StepNameEntry 13); small-data set: lev 310 (Menu_DrawContextHelpText 107, Menu_StepNameEntryAlt 43, Menu_StepSaveSelect 41) |
| `main/menu/save_slot.c` | cc1 `-G8`: lev 96 (Save_DrawSlotMetadata 96); maspsx `-G8`: lev 57 (Menu_SaveOverlayDraw 30, Save_DrawSlotMetadata 27); maspsx `--use-comm-section`: lev 13 (Menu_SaveOverlayDraw 11, Save_DrawSlotMetadata 2); FORCE_G0: lev 74 (Save_DrawSlotMetadata 74); small-data set: lev 127 (Save_DrawSlotMetadata 97, Menu_SaveOverlayDraw 30) |
| `main/menu/skills.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 177 (Menu_StepEquipConfirm 35, Menu_StepSkillList 27, Menu_OpenInventoryOrSwapView 20); maspsx `--use-comm-section`: lev 43 (Menu_StepEquipConfirm 7, Menu_StepSkillList 6, Menu_StepItemDetailPanel2 6); FORCE_G0: lev 4 (Menu_DeferredExec 4); small-data set: lev 177 (Menu_StepEquipConfirm 35, Menu_StepSkillList 27, Menu_OpenInventoryOrSwapView 20) |
| `main/menu/status_panels.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 243 (Menu_StepItemSelectScreen 44, Menu_SaveBgApplyFadeStep 35, Menu_HandleInput 27); maspsx `--use-comm-section`: lev 67 (Menu_SaveBgApplyFadeStep 14, Menu_StepItemSelectScreen 8, Menu_HandleInput 6); FORCE_G0: identical; small-data set: lev 243 (Menu_StepItemSelectScreen 44, Menu_SaveBgApplyFadeStep 35, Menu_HandleInput 27) |
| `main/menu/widget.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 797 (Menu_StepScrollCursor 133, MenuWidget_DrawList 109, MenuWidget_DrawListRow 74); maspsx `--use-comm-section`: lev 228 (MenuWidget_DrawList 37, MenuWidget_DrawListRow 29, Draw_FlushFrontBuffer 27); maspsx `--expand-div`: lev 42 (Draw_SetPrimCallback 22, MenuWidget_EaseNodePosition 20); FORCE_G0: identical; small-data set: lev 797 (Menu_StepScrollCursor 133, MenuWidget_DrawList 109, MenuWidget_DrawListRow 74) |
| `main/render/camera.c` | maspsx `--expand-div`: lev 70 (Render_StepCameraTransition 37, Render_StepGroupAnimations 33) |
| `main/render/font.c` | maspsx `--expand-div`: lev 30 (Render_LoadFontGlyphAlt 30) |
| `main/render/font_load.c` | cc1 `-fno-schedule-insns`: lev 13 (Render_LoadFontGlyph 13) |
| `main/render/frame_update.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 86 (Render_MoveGridCursor 86); maspsx `--use-comm-section`: lev 20 (Render_MoveGridCursor 20); maspsx `--expand-div`: lev 34 (Render_SetCDDCSlot 34); FORCE_G0: lev 89 (Render_SetCDDCSlot 59, CdRom_InitScreenState 19, Render_PrepareFrame 11); small-data set: lev 86 (Render_MoveGridCursor 86) |
| `main/render/mesh_entry.c` | cc1 `-fno-strength-reduce`: lev 93 (Render_LoadMeshEntry 93); cc1 `-fno-expensive-optimizations`: lev 16 (Render_TransformPoint 16) |
| `main/render/mesh_list.c` | cc1 `-fno-strength-reduce`: lev 191 (Render_DrawSpriteEntry 76, Render_ScrollEntries 52, Render_StepFade 23); maspsx `--expand-div`: lev 75 (Render_DrawSpriteEntry 36, Render_ScrollEntries 23, Render_StepFade 16) |
| `main/render/object.c` | cc1 `-G8`: lev 54 (Render_ColorEntity 32, Render_DrawObject 14, Render_TickObject 8); maspsx `-G8`: lev 3 (Render_TransformSkinnedVertices 3); maspsx `--use-comm-section`: lev 1 (Render_TransformSkinnedVertices 1); maspsx `--expand-div`: lev 48 (Render_ColorEntity 39, Anim_SetInterpRate 9); FORCE_G0: lev 121 (Render_SetEntityBlendMode 24, Render_DrawTexturedQuads 22, Render_DrawEntity 22); small-data set: lev 57 (Render_ColorEntity 32, Render_DrawObject 14, Render_TickObject 8) |
| `main/render/wireframe.c` | cc1 `-G8`: lev 57 (Render_DrawWireframeModel 57); maspsx `-G8`: lev 56 (Render_DrawWireframeModel 56); small-data set: lev 57 (Render_DrawWireframeModel 57) |
| `main/save/game_data.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 526 (Inv_DispatchCommand 83, Inv_RebuildWithBonusSlots 76, Inv_MergeStorageToSlot 54); maspsx `--use-comm-section`: lev 143 (Inv_DispatchCommand 19, Save_InitMetadataState 16, Inv_MergeStorageToSlot 16); maspsx `--expand-div`: lev 33 (Aya_StatQueryLevelAndSubLevel 17, Aya_StatQueryDistanceAndSubLevel 16); FORCE_G0: lev 52 (Inv_DispatchCommand 48, Save_GetCurrentMapNumber 2, Save_BuildHeader 2); small-data set: lev 526 (Inv_DispatchCommand 83, Inv_RebuildWithBonusSlots 76, Inv_MergeStorageToSlot 54) |
| `main/scene/room_load.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 63 (Scene_LoadRoom 63); maspsx `--use-comm-section`: lev 1 (Scene_LoadRoom 1); FORCE_G0: lev 305 (Scene_LoadEntityTextures 69, Scene_InitEntityPlayer 47, Scene_LoadRoom 41); small-data set: lev 63 (Scene_LoadRoom 63) |
| `main/script/actor_opcodes.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 207 (Akao_ProcessCommand 63, Render_CameraStepAngleFade 41, Render_CameraTrackRelativeOffset 28); maspsx `--use-comm-section`: lev 45 (Render_CameraStepAngleFade 10, Script_RunInvCommand 6, Render_CameraTrackRelativeOffset 5); FORCE_G0: lev 404 (Akao_ProcessCommand 82, Script_SetGteMatrix 48, Render_CameraTrackRelativeOffset 34); small-data set: lev 207 (Akao_ProcessCommand 63, Render_CameraStepAngleFade 41, Render_CameraTrackRelativeOffset 28) |
| `main/script/flow_opcodes.c` | cc1 `-G8`: same code, symbol table differs; maspsx `-G8`: lev 192 (Script_DispatchScriptCmd 59, Script_RunQueue 37, Entity_SelectFieldSetter 18); maspsx `--use-comm-section`: lev 49 (Script_RunQueue 10, Script_DispatchScriptCmd 9, Script_OpenSaveLoadMenu 5); FORCE_G0: lev 197 (Script_DispatchScriptCmd 66, Entity_SelectFieldSetter 26, Script_RunQueue 20); ASPSX_VERSION 2.70: lev 49 (Script_DispatchScriptCmd 49); small-data set: lev 192 (Script_DispatchScriptCmd 59, Script_RunQueue 37, Entity_SelectFieldSetter 18) |
| `main/script/node_actions.c` | cc1 `-G8`: lev 32 (Script_SpawnChildNode 32); maspsx `-G8`: lev 238 (Render_CameraTrackEntityZoom 46, Render_CameraSnapToEntity 41, Script_MoveTowardPoint 32); maspsx `--use-comm-section`: lev 45 (Script_SpawnChildNode 9, Script_MoveTowardPoint 6, Render_CameraTrackEntityZoom 6); maspsx `--expand-div`: lev 27 (Script_WaitAnimRange 27); FORCE_G0: lev 230 (Render_CameraTrackEntityZoom 54, Render_CameraSnapToEntity 39, Script_MoveTowardPoint 36); small-data set: lev 254 (Render_CameraTrackEntityZoom 46, Render_CameraSnapToEntity 41, Script_SpawnChildNode 40) |
| `main/script/node_pool.c` | cc1 `-G8`: lev 10 (Script_AllocNode 10); cc1 `-fno-strength-reduce`: lev 7 (Script_InitNodePool 4, Script_RelocBlock 3); maspsx `-G8`: lev 51 (Script_InitNodePool 13, Script_AllocNode 11, Script_RelocBlock 8); maspsx `--use-comm-section`: lev 17 (Script_InitNodePool 5, Script_AllocNode 4, Script_RelocBlock 2); FORCE_G0: lev 12 (Script_GarbageCollectNodes 12); small-data set: lev 57 (Script_AllocNode 17, Script_InitNodePool 13, Script_RelocBlock 8) |
| `main/script/sfx_queue.c` | maspsx `-G8`: lev 30 (Scene_UpdateEntityList 22, Script_QueueFieldSfx 6, Script_ClearSfxTable 2); maspsx `--use-comm-section`: lev 7 (Script_QueueFieldSfx 3, Scene_UpdateEntityList 3, Script_ClearSfxTable 1); FORCE_G0: identical; small-data set: lev 30 (Scene_UpdateEntityList 22, Script_QueueFieldSfx 6, Script_ClearSfxTable 2) |
| `main/script/system_opcodes.c` | maspsx `-G8`: lev 67 (Entity_InitWithTimeout 16, Script_WaitEntityFlags8 11, Script_WaitFrameOrDraw 8); maspsx `--use-comm-section`: lev 24 (Entity_InitWithTimeout 5, Script_WaitEntityFlags8 4, Script_WaitFrameOrDraw 3); FORCE_G0: identical; small-data set: lev 67 (Entity_InitWithTimeout 16, Script_WaitEntityFlags8 11, Script_WaitFrameOrDraw 8) |
| `main/script/value_ops.c` | maspsx `--expand-div`: lev 41 (Script_EvalExpr 41) |
| `overlays/fx_common/effect_init.c` | cc1 `-fno-expensive-optimizations`: lev 28 (func_80196498 28) |
| `overlays/fx_common/motion_endpoints.c` | cc1 `-fno-schedule-insns`: lev 56 (FxCommon_SetMotionEndpoints 56) |
| `overlays/fx_common/camera.c` | cc1 `-G8`: lev 27 (func_80191EFC 15, func_80191E30 12); maspsx `-G8`: identical; maspsx `--use-comm-section`: identical; FORCE_G0: lev 536 (func_80191854 141, FxCommon_CheckFourBoundsWithMargin 103, FxCommon_CheckBoundsWithMargin 51); small-data set: lev 27 (func_80191EFC 15, func_80191E30 12); maspsx `--expand-div`: lev 165 (FxCommon_CheckFourBoundsWithMargin 88, FxCommon_CheckBoundsWithMargin 44, func_8018F55C 33) |
| `overlays/mdec_diag/decdct.c` | `ASSEMBLER: GNU`: lev 18 (MdecDiag_Reset 10, DecDCTReset 2, DecDCTPutEnv 2) |
| `overlays/boot_display/sprites.c` | maspsx `--expand-div`: lev 22 (BootDisplay_StepImageSprite 22) |
| `overlays/menu_memcard/page_flip.c` | cc1 `-fno-cse-skip-blocks`: lev 12 (MenuCard_FlipScreenPages 12) |
| `overlays/menu_memcard/list_screen.c` | cc1 `-fno-cse-skip-blocks`: lev 71 (func_801909B4 59, func_80190660 12) |
| `overlays/room_lib/dropped_flare.c` | maspsx `--expand-div`: lev 63 (RoomEffect_DroppedFlareParticle 63) |
| `overlays/room_lib/line_burst.c` | maspsx `--expand-div`: lev 36 (RoomEffect_LineBurstController 36) |
| `overlays/room_lib/spark_attacks.c` | maspsx `--expand-div`: lev 37 (RoomEffect_BeamSparkController 37) |
| `overlays/room_lib/trail_burst_attacks.c` | maspsx `--expand-div`: lev 66 (RoomEffect_SeekingTrailParticle 66) |
| `overlays/room_lib/grab_attack.c` | maspsx `--expand-div`: lev 17 (RoomEffect_RisingSprayController 17) |
| `overlays/room_lib/mirror_model.c` | maspsx `--expand-div`: lev 29 (RoomEffect_ModelUpdate 29) |
| `overlays/room_lib/motion_particles_2.c` | cc1 `-fno-strength-reduce`: lev 66 (RoomEffect_DrawMotionParticles 66) |
| `overlays/room_lib/orbit_set.c` | cc1 `-fno-strength-reduce`: lev 179 (RoomEffect_UpdateOrbitBurst 38, RoomEffect_DrawOrbitBurst 33, RoomEffect_InitOrbitParticles 30) |
| `overlays/room_lib/actor_classes.c` | maspsx `--expand-div`: lev 194 (RoomLib_HandlerE 58, RoomLib_HandlerC 33, RoomLib_AdvanceArcToTargetY 28) |
| `overlays/room_lib/floor_walker_class.c` | maspsx `--expand-div`: lev 10 (RoomLib_UpdateHandlerG 10) |
| `overlays/room_lib/window_class.c` | maspsx `--expand-div`: lev 20 (RoomLib_WindowHandler 20) |
| `overlays/room_m023_theater/m023.c` | maspsx `--expand-div`: lev 16 (func_8018F710 16) |
| `overlays/room_m256_museum/m256.c` | maspsx `--expand-div`: lev 14 (func_80192680 14) |
| `overlays/room_m273_liberty/RoomM273_PulseSpriteCallback.c` | cc1 `-fno-schedule-insns`: lev 24 (RoomM273_PulseSpriteCallback 24) |
| `overlays/room_m273_liberty/m273.c` | maspsx `--expand-div`: lev 30 (func_80197BBC 17, func_80192F5C 13) |
| `overlays/room_m318_chrysler/m318.c` | maspsx `--expand-div`: lev 22 (func_801955E4 22) |
| `overlays/room_m349_cruiser/m349.c` | maspsx `--expand-div`: lev 47 (func_8018F7C4 47) |
| `overlays/room_m350_cruiser/RoomEffect_ConfiguredPulseCallback.c` | cc1 `-G8`: lev 7 (func_80196C64 7); maspsx `-G8`: identical; maspsx `--use-comm-section`: identical; FORCE_G0: lev 14 (func_80196C64 14); small-data set: lev 7 (func_80196C64 7) |
| `overlays/room_m350_cruiser/m350.c` | maspsx `--expand-div`: lev 56 (func_80195CE0 21, func_8019421C 18, func_80192508 17) |
| `overlays/room_m350_cruiser/m350_callbacks.c` | maspsx `--expand-div`: lev 52 (func_80198ABC 18, func_801992A0 17, func_80197B98 17) |
| `overlays/scene_e18_cruiser/e18_emitters.c` | maspsx `--expand-div`: lev 17 (func_801935F0 17) |
| `overlays/scene_e19_cruiser/e19.c` | maspsx `--expand-div`: lev 133 (func_80192F9C 133) |
| `overlays/scene_e20_cruiser/SceneE20_HoverOrbController.c` | cc1 `-ffixed-10`: lev 6 (SceneE20_HoverOrbController 6); cc1 `-ffixed-11`: lev 6 (SceneE20_HoverOrbController 6); cc1 `-ffixed-15`: lev 6 (SceneE20_HoverOrbController 6); cc1 `-ffixed-21`: lev 74 (SceneE20_HoverOrbController 74); cc1 `-ffixed-22`: lev 74 (SceneE20_HoverOrbController 74); cc1 `-ffixed-23`: lev 74 (SceneE20_HoverOrbController 74); cc1 `-ffixed-24`: lev 6 (SceneE20_HoverOrbController 6); cc1 `-ffixed-25`: lev 6 (SceneE20_HoverOrbController 6); maspsx `--expand-div`: lev 69 (SceneE20_HoverOrbController 69) |
| `overlays/scene_e22_liberty/e22.c` | maspsx `--expand-div`: lev 44 (func_80198348 44) |
