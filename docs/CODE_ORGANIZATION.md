# Code organization contract

This repository is a long-lived reconstruction of the original program, not a
collection of independent decompilation puzzles. A byte match is necessary,
but it is not sufficient evidence that source boundaries, declarations, names,
or types are correct.

## Evidence order

Use evidence in this order when placing or combining source:

1. executable addresses, section layout, relocations, and manifest order;
2. known object or archive boundaries and matches to Psy-Q or AKAO libraries;
3. shared data, call relationships, and static state;
4. responsibility of the code and its callers;
5. existing filenames, which are hints rather than evidence.

Do not infer an original object boundary merely because splat emitted one
function or because one function is convenient to permute in isolation. Do not
merge non-contiguous code or cross a strong responsibility boundary merely to
reduce the file count.

## Directory taxonomy

- `src/main/<subsystem>/` contains code resident in `SLUS_006.62`.
- `src/overlays/<binary>/` contains code whose link address and lifetime belong
  to that overlay. Code shared by several overlays stays duplicated until
  binary evidence identifies a linked library or a reproducible generation
  rule.
- `src/overlays/room_lib/` holds such a linked library once. The room
  library (`actor_classes.c`) is the same run of 53 functions, in the
  same order and with the same jump tables, in 125 room and scene overlays.
  Each overlay's manifest names the unit as a `../room_lib/actor_classes`
  subsegment, and `make overlay-build` compiles it into that overlay's own
  build directory, so every overlay still links its own copy at its own
  address. Eight rooms link a sixth class right after it
  (`floor_walker_class.c`); whether that class was part of the same
  object cannot be told from the binary, so it is a unit of its own.
  Smaller effect objects shared by a few rooms live there the same way
  (`dropped_flare.c`, `twelve_elements.c`, ...): each
  is a run of functions in the same order in every instance, with a rodata
  block (seed initialisers, jump tables) that is contiguous and in the same
  order too. Data the code reads from the room (packet templates, colour
  tables) stays in each room's manifest under one name per role, given in
  the room's symbol file. The methods of a room module's class (the run in
  front of each effect module: `RoomLib_PlantScript`, `RoomLib_Spawn6`,
  `RoomLib_RegisterDrawList`, `RoomLib_RegisterPairedTables`,
  `RoomLib_CloseTarget`) are one unit per function (`class_*.c`), because
  rooms replace single methods with their own; see
  `include/pe1/room_module.h`. A module whose own class always precedes
  it shares one unit with it (`orbit_set.c`); the units and what keeps
  the rest apart are listed under "Room library units" below.
- `include/pe1/` owns game ABI, shared data structures, and declarations.
- `include/pe1/akao/` owns the AKAO command, queue, track, and SPU interfaces.
- hardware and SDK declarations belong in their subsystem header rather than
  being repeated in C files.

The subsystem list, the target directory for every current directory and the
naming rules are in [CONVENTIONS.md](CONVENTIONS.md).

Names such as `func_800...`, `misc`, `task7`, and `spu4` explicitly mean
"identity or module boundary not established". They are migration state, not a
stable naming convention. `make organization-check` prevents the address-name
and placeholder-name counts from increasing independently in main and overlays.

## Translation-unit rules

A merge or split is accepted only when all of the following hold:

- manifest order and addresses remain unchanged;
- the proposed unit is contiguous, unless independent object evidence proves
  otherwise;
- shared declarations and private state have a clear owner;
- `make check` remains byte-identical after a clean rebuild;
- configured overlays touched by the change pass `make overlay-check`;
- the evidence and any remaining uncertainty are recorded in the commit or in
  maintained documentation.

Large cohesive units are acceptable. Function count and aesthetics alone are
not reasons to invent source boundaries.

`src/` has a strict one-to-one relationship with committed manifest entries.
The one exception is a shared library unit under `src/overlays/room_lib/`:
many overlay manifests may list it, each at most once.
The check uses files present on disk as well as Git's index: an untracked C file
cannot enter a local build, and a manifest promotion cannot pass unless its
source will exist in a fresh clone.
Non-matching experiments are not tracked. A C file enters `src/` only in the
same change that configures its range as `c` and passes the binary check.

## Declaration ownership

New file-scope `extern` declarations in `.c` files are forbidden by the debt
gate. Put public or cross-unit declarations in the narrowest appropriate
header. Keep declarations private only when every use and definition are in a
single translation unit.

Existing declarations are migration debt tracked separately for main and
overlays. Moving a declaration must preserve its exact signedness, width,
qualifiers, and function prototype; do not "improve" an uncertain ABI while
centralizing it.

A symbol that units read with different widths or access patterns still gets
one declaration: a union with one member per load width (`RoomFloorY` in
`pe1/room_floor.h` has `y` for `lh` and `raw` for `lhu`), declared as an
incomplete array when a `-G8` unit must keep the absolute address.

### Per-file flag conflicts

A merge that one neighbour's cc1 option seems to forbid is usually a C-shape
question, not a real conflict:

- G0341: `Boot_InitSubsystems` only needed `-fno-schedule-insns2` for its
  prologue store order; starting its flush loop after `Draw_InitGpuHwRegs` as
  a `for` loop gives the retail order under the default schedule.
- G0835: the background scroll step only needed strength reduction for the
  loop pointer copy it creates; walking the entry pointer by hand inside a
  guarded `do`/`while`, with the camera pointer taken before it, matches under
  `-fno-strength-reduce`.

### Overlay translation units (src/overlays, 2026-10-12)

The overlays' own C (outside `room_lib/`) went from 432 files to 62 (97
until the `boot_display` and `menu_memcard` merges of 2026-10-13). None of
the 149 address-named files is left: the last, the second instance of the
event-flag template in `menu_memcard` (the first is in `video.c`), is now
part of `menu_memcard/video_playback.c`, where its function keeps the
address-named `func_8012B1F0`. The others are named by role, and the functions
they hold take that name when the overlay has only the one copy. Adjacent units of one overlay were merged when nothing else
lies between them in the manifest (no shared `room_lib` unit, data, or foreign
rodata between the members' rodata) and their per-file markers agree.
`--expand-div` is not a difference: every default overlay unit compiles
byte-identically with it, so a run that contains one `--expand-div` member
becomes one unit with that marker (since 2026-10-12 `--expand-div` is the
default and no unit writes it). Each merge was checked with
`make overlay-check` of its overlay; `tools/reorg/merge_overlay_units.py`
does the mechanical part (hoisted includes and types, one folded
`<unit>_externs.h`, `#undef` of each member's template macros, renamed
clashing local types, prototypes taken from the definitions).
`room_lib/` is described in its own section below.

The main run of a room or scene is `<short>.c` (`scene_e08_chrysler/e08.c`),
a grab-trigger class prelude is `<short>_class.c`, and other runs are named
after what their functions do (`fx_common/camera.c`,
`menu_memcard/glint_effects.c`). Units that stay separate, and why:

- Copied code with its own link address: `boot_display` and `menu_memcard`
  are built from segments whose functions keep the link VMA of the place they
  were copied from, so a segment is at most one unit. Only segments with
  several functions merged (`boot_display/{sprites,playback}.c`,
  `menu_memcard/{video,glint_effects,video_open_step,fade}.c`).
- Flag singletons: `fx_common` (`motion_endpoints.c` needs
  `-fno-schedule-insns`, `effect_init.c` `-fno-expensive-optimizations`,
  see below), `menu_memcard` (`-fno-cse-skip-blocks` twice),
  hold functions whose local jumps (`j`, jump tables) resolve at the link
  address of the place they were copied from, so a unit cannot span two
  link bases. A unit without local jumps (no `.text`/`.rodata` relocation of
  its own) reveals no base and sits in the segment of its neighbours.
  These two overlays use lower snake-case role names, like `src/main`
  (`tools/reorg/maps/step9_overlay_shared_units.csv`):
  - `boot_display` runs at 0x80122FA4 throughout: the switch tables, the
    callback tables of the data block (0x80123E08 is
    `BootDisplay_InitDisplayResources`, 0x80125444
    `BootDisplay_CallInitSystem`) and the calls between units name
    addresses of that base, and the glyph-width lookup only lands inside
    the width tables with it. The units without local jumps had been put at
    0x80122EA4; they moved to 0x80122FA4 with the data block, the
    `BootDisplay_SetCdFilter`/`BootDisplay_RecordReadPosition` symbols and
    three address-named helpers (now `BootDisplay_QueueFadeSprite`, `BootDisplay_PresentSpriteFrame`,
    `BootDisplay_IssuePlaybackRead`). The units follow the two callback tables:
    `text_screen.c` (main loop, text block, glyph lookups; one unit, so the
    4-byte font-table pad is the alignment of the glyph jump table),
    `sprites.c` (the sprite-mode callbacks), `image_display.c` (the
    image-mode callbacks, from `BootDisplay_InitDisplayBuffers` on),
    `playback_stop.c`, `playback_file.c` and `playback.c`.
    `playback_file.c` (`BootDisplay_InitPlaybackFile`) resolves its local
    jumps 0xE8 below the overlay base, so it stays a unit of its own and
    `playback_stop.c` before it cannot join `playback.c` after it.
  - `menu_memcard` has four link bases: `video.c` 0x80120D00, the effect
    callbacks 0x801EB7F8 (`glint_effects.c`, `image_effects.c`), the screen
    and the second video player copy 0x801887F0 (`page_flip.c`,
    `image_nodes.c`, `list_screen.c`, `video_playback.c`) and the fade
    code 0x801887A4 (`fade.c`; 0x4C, the size of the copied event-flag
    handler, below the player's). Units without local jumps that had sat at
    the 0x80120D44 load address (`MenuCard_ResetDisplayState`,
    `MenuCard_StopVideo`, `MenuCard_InitFadeForNode`, the old `fade.c`)
    joined their neighbours' base; callers still reach them by their load
    addresses through pinned names. `glint_effects.c` and
    `image_effects.c` (which ends with the glint origin hooks retail places
    after the image effects) share a base and markers and are split by role
    only.
- Flag singletons: `fx_common` (`-fno-schedule-insns`,
  `-fno-expensive-optimizations`; the two `-G8` units are `render_op.c`),
  `menu_memcard` (`-fno-cse-skip-blocks` in `page_flip.c` and
  `list_screen.c`; `MenuCard_RedrawImageNodes`, the first function of
  `image_nodes.c`, is lev 45 under that flag, so `image_nodes.c` joins
  neither neighbour; its other four functions compile identically with
  and without it),
  `room_m273_liberty` (`RoomM273_PulseSpriteCallback`, `-fno-schedule-insns`,
  which also leaves `RoomEffect_PhasedArcSpriteFlow` alone after it),
  `room_m350_cruiser` (`RoomEffect_ConfiguredPulseCallback`, `-G8`) and
  `scene_e20_cruiser` (the `-ffixed-*` hover orb splits its three units).
  `docs/COMPILATION_FLAGS.md` has the lev of each unit without its flag.
- Between shared units: `scene_e08_chrysler/SceneE08_ModuleStart` and
  `room_m123_nypd/RoomM123_SelectSlotPointer`. `scene_e14_chrysler`'s only
  unit was a copy of the room library's script-argument entry and now links
  the shared `room_lib/script_args` with its words named
  `g_RoomScriptArgs`, so that scene has no C of its own.
- `mdec_diag` (2026-10-13): the LIBPRESS DCT object is `decdct.c`,
  `mdec_wait.c` and `mdec_status.c`; the decoder-table builder is
  `vlc_build.c` and the two SDK assembler objects are `vlc.c` and `vlc_c.c`,
  after their PsyQ objects VLC and VLC_C (each sits in its own segment
  between data). `DecDCTReset` used to be GCC 2.8.1 and four neighbours
  GNU as, but every function from `DecDCTReset` to the status read compiles
  to the same instructions with GCC 2.7.2 and plain GNU as (GNU as fills the
  return delay slot that made `DecDCTReset` look like GCC 2.8.1), so the
  seven units from `DecDCTReset` to `MdecDiag_ReceiveOutput` are one
  `ASSEMBLER: GNU` unit. The two busy waits stay apart: `mdec_wait.inc`
  declares a global register variable on `$a0`, which GCC 2.7.2 rejects
  after any function definition of the unit ("global register variable
  follows a function definition"), and once declared it keeps `$a0` from
  every later function, so the status read and timeout handler
  (`mdec_status.c`) score lev 3 after the waits (`MdecDiag_HandleTimeout` loads the
  status register into `$a1`).
- `fx_common` (2026-10-13): `camera_nodes.c` and `render_op.c` are one
  K8 unit, `camera.c`. Under cc1 `-G8` the camera aim function scored lev 16
  because the 8-byte angle vector `g_FxCommonCameraAngles` became small data and lost
  its address register; declared as an incomplete array it keeps the
  register and every function of both units is lev 0. Merging also exposed
  that the render ops' definitions disagreed with their prototypes in
  `fx_common_motion.h`; both now take the sound handle and the position
  words the effect loop passes (`Akao_Cmd_A0`/`A2` take an `int` handle).
  The rest stays apart, each lev with its neighbour's settings:
  `effects.c` lev 817 under `-fno-schedule-insns`, `motion_endpoints.c`
  lev 56 without it; `camera_path.c` lev 66 under `-fno-schedule-insns` and
  lev 32 under `-fno-expensive-optimizations`; `effect_init.c` lev 28
  without that option, `model_buffers.c` lev 288 with it; `effects.c` lev
  143 under K8.
- `fx_field` has no C of its own: it links the shared
  `room_lib/RoomEffect_OrbitFlare` and `room_lib/RoomLib_SetArgs3`.
- Data between runs, resolved: `room_m256_museum`'s swing and effects runs
  are one `m256.c`. The 0x24 bytes between their jump tables were the
  effects' stack templates (`D_8018F208`..`D_8018F220`), now `const`
  definitions placed between the two runs: GCC writes a function's jump
  tables when it finishes the function and a const global where it is
  defined, so the templates land after the swing tables and before the
  effects tables, and the 8-aligned effects tables take the 4-byte pad
  retail has. In a unit of its own the effects run could not reproduce
  that pad (the link shrank by 4).
- Two type views of one symbol, resolved: in `room_m273_liberty` the
  poll/reset template declared `D_8019AF04` as a `RoomFxSeed8` seed and the
  boss code as the `RoomM273DropQueue` that starts with it. The queue's
  first member is now that seed (`hit`), the template copies
  `D_8019AF04.hit`, and its including units declare the words they name,
  so the main run, the rising sprite pools and the boss sway are one
  `m273.c`.
- Probed, still split: `room_lib/motion_particles{,_2,_3}`.
  Under the default flags `RoomEffect_DrawMotionParticles` is lev 66 (loop
  strength reduction turns the lookup index and the particle cursor into
  pointers that retail computes per iteration); under
  `-fno-strength-reduce` the other units lose the reductions retail has:
  `RoomEffect_MotionParticlesInit` lev 24, `RoomEffect_SeedMotionParticles`
  lev 21, `RoomEffect_UpdateParticleMotion` lev 64. Writing the setup loop
  with the reduced byte offset by hand brings `RoomEffect_MotionParticlesInit`
  to lev 9; the rest is loop-invariant hoisting (`-64` leaves the loop once
  the strength-reduction register is gone). The split matches what the
  binary shows, two flag sets, until one C shape reproduces both.

### Room library units (src/overlays/room_lib, 2026-10-13)

`room_lib/` went from 90 files (53 units, 12 templates and 25 headers) to
54 (48 units, 3 templates, 3 headers). The units are named like the
resident ones, lower snake case by role (`orbit_flare.c`,
`class_spawn.c`, `script_args.c`); function names did not change, and
`tools/reorg/maps/step10_room_lib_units.csv` maps every old file to its
unit. A shared unit exists because the same run of functions, in the
same order and with the same rodata, appears in several overlays; it is
compiled into each of them with the unit's own markers, so two units
became one only when every overlay that links one links the other
directly after it, with nothing (code, data or foreign rodata) between
them, and one marker set compiles both. Merged, all 190 overlays
byte-identical:

- `drop_field.c`: the drop field's class and its effect set (room_m174,
  m348, m383);
- `ground_eruption.c`: the eruption's class and the eruption (room_m141,
  m146, m153, m154, m328, scene_e02, e04, e05);
- `orbit_set.c`: the orbit set's class and the set (room_m186, m187,
  m385, m388, m389); the class compiles the same under the set's
  `-fno-strength-reduce`;
- `three_particles.c`: the three particles' class and the particles
  (eleven Central Park and Chrysler rooms);
- `class_register_active.c`: the two status-3 registration methods of the
  glow orb rooms (room_m034, m174, m383).

The templates with one instance were written out where they are used:
`floor_walker.inc` and `transform_scale.inc` in `floor_walker_class.c`,
and the seven glint, orbit-spin and jitter-spark templates in
`scene_e19_cruiser/e19.c` under that scene's function names, with their
parameter declarations in `e19_externs.h`. The per-unit `_externs.h`
headers are gone: their declarations were already in the unit's
`include/pe1` header or `room_lib.h` (`mirror_model.c`, `paired_glow.c`,
`twelve_elements.c`, `three_particles.c`, the bounce template's colour,
which each includer defines), or moved there (`motion_particles.h` for
the motion particle set's field engine calls and the folded shared
state view, `room_spark_attacks.h` for the capture block of
`capture_frame_sample.c`, `room_lib.h` for the timed spawns' ramp table,
`room_floor_walker.h` for the scratchpad scale words). The effect
sequence's private record types moved from their header into
`effect_sequence.c`.

Still separate, and why:

- The class methods (`class_nop0.c`, `class_plant_script.c`,
  `class_spawn.c`, `class_register_draw_list.c`,
  `class_register_tables.c`, `class_register_tables_gated.c`,
  `class_close_target.c`, `class_nop6.c`): each is linked by a different
  set of rooms (21, 43, 46, 37, 32, 5, 41 and 16), because a room links
  only the methods it does not replace.
- Same rooms, not adjacent: `glow_orb_setup.c` and `glow_orb.c` (the
  shared spawn, registration and close methods lie between them);
  `floor_walker_class.c` and `timed_spawn.c` (room_m152 links
  `dropped_flare.c` between them); `spark_attacks.c` and
  `capture_frame_sample.c` (`script_arg.c` and `script_slot.c` between);
  `homing_projectile.c` and `ground_eruption.c` (other modules, and in
  some rooms the room library, between; the projectile's seeds are a
  data unit, `homing_projectile_seeds.c`).
- Different rooms: `class_update_callback.c` (room_m256, m273, scene_e19)
  and `grab_await_stance.c` (only m273 and e19); `line_burst.c` (14 rooms)
  and `script_slot.c` (17, three of them after `script_arg.c` instead).
- The scene script entries (`script_arg.c`, `script_args.c`,
  `script_slot.c`, `script_task_var.c`, `script_arg_pair.c`) are room
  glue that follows whichever effect module a room links last:
  `script_args.c` follows the orbit flare, the homing flash or the sound
  burst. `script_arg_pair.c` follows `spark_barrage.c` in both of its
  rooms and would merge byte-identically, but it is kept with its family
  rather than folded into the effect module.
- `motion_particles{,_2,_3}.c`: the particle draw needs
  `-fno-strength-reduce` and the setup and update loops must not have it
  (see "Probed, still split" above).
- Templates still instantiated more than once: `bounce_render.inc`
  (`line_burst.c`, `spark_attacks.c`), `fan_sweep_spark.inc`
  (`room_m318_chrysler/m318.c`, `fieldfx/sprite_effects.c`) and
  `poll_reset_actor.inc` (twice in `room_m273_liberty/m273.c`).

### Resident translation units (src/main, 2026-10-12)

`src/main` outside `psyq/` went from 339 files to 103. Units were first moved
into the directory of their module (`docs/reorganization_mapping.csv`,
`tools/reorg/maps/step1c_*.csv`), then merged when they are adjacent in the
manifest, nothing foreign lies between their rodata, and one marker set
compiles every member to the same instructions: the READY modules of
`docs/REORGANIZATION_PLAN.md` and the compatible parts of the PARTIAL ones.
`tools/reorg/reorg_merge.py` concatenates the units in manifest order (one
include set, the tentative COMMON definitions of the words retail reaches
through `$gp` in one block after the includes, private `*_externs.h`
declarations moved to a shared header, `Old.c:NN` references re-pointed);
`tools/reorg/marker_probe.py` compares every function with the old unit
objects before `make check` and `make overlay-check-all` decide.

File names are lower snake case and name the role of the unit inside its
directory, without the directory or the function prefix (`fieldfx/glow.c`,
`script/flow_opcodes.c`, `menu/inventory.c`, `battle/setup.c`). The old
function-style names (`Battle_EncounterFlow.c`) and their new paths are in
`tools/reorg/maps/step9_snake_case_names.csv`, the unit that holds each
file of the original layout is the `current` column of
`docs/reorganization_mapping.csv`, and `make organization-check` counts
names that break the rule
(`unconventional_files`). A unit whose functions all belong to another
subsystem sits in that subsystem's directory even where the binary places
it among other code: the main update between field collision and the hit
and drop table is `menu/main_update.c`, the hit, drop-table and timer unit
after it `entity/combat.c`, the render pipeline set-up between the boot
units `draw/pipeline.c`, the item use panel opener between the inventory
units `menu/item_use_panel.c`, the font glyph units and the wireframe model
in the draw run `render/font_load.c`, `render/font.c` and
`render/wireframe.c`, and the GNU-as square-root approximation after the
frame presentation `math/sqrt_approx.c`.

The marker sets that unified the most units:

- K8: cc1 `-G8`, MASPSX `-G8 --use-comm-section`, `MASPSX_FORCE_G0`, plus a
  COMMON definition for every word one of the old units reached through
  `$gp`. Old per-unit `-G0`, `-G1`, `-G2`, `-G4` and `-G4096` values were
  only ways to get the same `$gp` set; with FORCE_G0 only COMMON words are
  `$gp`, and every other extern stays absolute.
- Soft flags taken as a union where they are neutral for the other members:
  `--expand-div`, `--dont-expand-li`, `-fno-cse-skip-blocks`,
  `-fno-expensive-optimizations`, `-fno-strength-reduce` (node pool),
  `ASPSX_VERSION` 2.70/2.77. `-fno-force-mem` turned out unnecessary for
  `Battle_FinalizeAttackResult` and the geometry control entries.
- Three one-line C changes from `docs/REORGANIZATION_G_FLAGS.md`: the name
  entry clears the game-state flags through `D_800B0CD8` (#29), the player
  palette sets the story-day flag through `&g_GameState.story_day_flags`
  (#14), and `g_RenderCounter` is declared unsized (#19).

Data between the units: `main/rodata_0009AC`, `main/rodata_001324`,
`main/rodata_001080` and `main/rodata_00196C` were four zero bytes each, the
pad GCC puts before an 8-aligned jump table when both units are one object;
the merged objects emit them, so the entries are gone (`--absorb`).
`main/rodata_0015FC` is real data, now const definitions at the end of
`battle/status_hud.c` (status symbol templates and window bounds): GCC
writes them after the status primitive constants and before the jump
tables, as retail has them, and defining them after their users keeps the
users' byte-aligned struct copies.

Units that stay separate, and why:

- `pm/process.c` is not part of `pm/disc_tables.c`:
  the disc table loaders include `pe1/cd_poll.h`, whose `extern inline`
  `CdRom_PollReady` they inline, while `Scene_LoadRoomAssets` in the process
  manager unit calls it (lev 56 when the two share a translation unit). This
  is binary evidence of an original file boundary.
- Word clashes (a word one unit reaches through `$gp` and another reads
  absolutely): `entity/frame_update.c` with
  `entity/allocation.c` (`g_FieldActorListHead`,
  `g_PlayerEntity`; `Scene_UpdateEntityPositions` lev 97) and
  `battle/enemy_slots.c` after `battle/encounter.c`
  (`g_BattleEquipStateBlock`; `Entity_AllocSlot` lev 11).
- Section padding: `battle/enemy_movement.c` after
  `battle/turn.c`. As one object the 8-aligned turn-flow rodata
  is padded after the movement's 11 bytes and the link grows by 4; the
  per-function probe cannot see this, `make check` does.
- Per-unit inline helpers: `menu/item_use_panel.c` and
  `item/armor_capacity.c` each define their own `LookupTrackedItem`
  shape, so the use-panel opener stays between `item/swap.c` and
  `item/armor_capacity.c`.
- Flags the neighbours do not tolerate: `fieldfx/angle.c`,
  `draw/color_quad.c` and
  `render/font_load.c` (`-fno-schedule-insns`),
  `asset/unload_table.c` (`-fno-force-mem`, lev 1 without),
  `menu/equip_screen.c` (does not match under the item detail
  panel's `-fno-expensive-optimizations`), `entity/pool.c`
  (`-fno-strength-reduce`, which `Scene_LoadMap` does not take),
  `render/mesh_entry.c` and `render/mesh_list.c`, the
  primitive queue in `draw/` (`Draw_InitGpuHwRegs` and
  `Draw_FlushPrimQueue` are lev 4 under each other's set) and the `-g3`
  battle units (`menu/main_update.c`, `battle/phase.c`, which
  also isolates `battle/player_turn.c`, and the drop table, hit application
  and timer update, which do compile as one `-g3` unit).
- Open small-data shapes from `docs/REORGANIZATION_G_FLAGS.md`:
  `script/value_ops.c` with `script/node_actions.c` (#0) and
  `draw/pipeline.c` (#20, also a gprofile boundary).
- Source policy: the GNU-as units (`math/fixed_mul.c`,
  `math/fixed_round.c`, `draw/prim_color.c`,
  `math/sqrt_approx.c`) stay alone.

### PsyQ objects split over several units (src/main/psyq)

Each `psyq/<library>/` unit reproduces one SDK object, or a contiguous part
of one, and is named after the object (`sys.c`); the parts of an object that
stays split are numbered in address order (`sys_2.c`, `sys_3.c`, ...).
Parts are merged whenever every function of the merged unit compiles to its
retail words under one marker set, so the splits that remain record a
toolchain limit, not an original source boundary: PsyQ compiled each object
once, but the stock compilers here reproduce some of its functions only
with GCC 2.7.2 and its neighbours only with GCC 2.8.1, or need an option
that breaks the neighbour. Two retail traits mark the limit:

- an unfilled return: `lw ra; addiu sp; jr ra; nop`. GCC 2.7.2 emits it
  whenever no instruction can go into the epilogue's delay slot; GCC 2.8.1
  expands the epilogue as RTL and always moves the stack adjustment into
  the `jr ra` slot. Such a function can only be GCC 2.7.2.
- several fields of one global reached through one base register
  (`la s0, sym` and `sw x, 4(s0)` ...), often across calls. GCC 2.8.1's
  cse turns repeated constant addresses into that form; GCC 2.7.2 does the
  opposite and folds every register-plus-offset address it knows to be
  constant into an absolute `lui/sw`. GCC 2.7.2 keeps the base only when
  cse does not know the pointer is constant: a store through a pointer
  variable without an offset, an address in a variable that is reassigned
  elsewhere, or a block cse does not reach along its jump-following path
  (a loop boundary). The merges below use those shapes; where a function
  needs the base for many fields across calls no plain C shape was found.

An object whose functions show both traits cannot be reproduced under one
stock compiler; the table gives the best single marker set and its lev.

The evidence is a marker probe: every part is compiled under the marker set
of each sibling (and with no markers) and compared, instruction word by
instruction word with relocations, and section by section for data, with
its retail-matching object. A run of neighbours with one common lev-0 set
is merged and then confirmed by `make check`.

LIBAPI PATCH and LIBCARD PATCH are a policy split instead: their assembler
parts (`PSYQ_ASM_FUNCTION`, counted as `original_asm` units) may not share
a unit with C functions. LIBSPU S_R and S_W are separate objects; the
provenance file lists both at both addresses because their signatures are
byte-identical.

| object | units (before) | units now | why the split stays |
|---|---|---|---|
| LIBGPU SYS | 17 | 11 (`sys.c` .. `sys_11.c`) | parts 1, 3, 6, 8 and 10 need GCC 2.8.1 with `-mno-split-addresses` (base registers); 2, 4, 7, 9 and 11 need GCC 2.7.2 (unfilled returns in `Gpu_RestoreDmaCallback` and others); part 5 needs `-O1`. Best single set GCC 2.8.1 with GNU as and `-mno-split-addresses`, lev 115; GCC 2.7.2 with GNU as lev 412 |
| LIBCD SYS | 6 | 2 | `sys.c` GCC 2.8.1: GCC 2.7.2 tests the `CD_initvol` result in `$v0`, retail copies it to `$v1` first (`CdReset` lev 5 under GCC 2.7.2 with GNU as, the set of `sys_2.c`) |
| LIBCD BIOS | 9 | 7 | parts alternate between GCC 2.7.2 (2, 4, 5) and GCC 2.8.1 (1, 3, 6, 7); parts 2, 3, 4 and 6 each need their own `-fno-expensive-optimizations` or scheduling option. Best single set GCC 2.8.1 with GNU as and `-mno-split-addresses`, lev 92; GCC 2.7.2 with GNU as lev 222 |
| LIBCD C_011 | 3 | 2 | StCdInterrupt and mem2mem GCC 2.7.2 (an unfilled return), dma_execute GCC 2.8.1 with `-fno-schedule-insns`. Best single set GCC 2.7.2 with GNU as, lev 13 (`dma_execute`) |
| LIBETC INTR | 3 | 2 | the callbacks GCC 2.7.2 (eight unfilled returns, lev 16 under GCC 2.8.1), the interrupt manager GCC 2.8.1 (base register, lev 76 under GCC 2.7.2 with GNU as) |
| LIBAPI PATCH | 2 | 2 | `_patch_pad` is SDK assembler (policy) |
| LIBCARD PATCH | 4 | 4 | parts 1 and 3 are SDK assembler (policy) |
| LIBDS DSSYS_1 | 11 | 10 | neighbours alternate between GNU as with GCC 2.7.2 and GCC 2.8.1, some with their own scheduling option; the GCC 2.8.1 parts keep a base register in `$s0` across calls (under GCC 2.7.2 with GNU as: `DS_stop` lev 18, `LIBDS_DSSYS_1_text_E10` 17, `tipDsSystem` 26, `DS_reset_members` 49, the four `text_368`..`text_7FC` helpers 181; best single set GCC 2.8.1 with GNU as and `-mno-split-addresses`, lev 87) |
| LIBDS DSSYS_2 | 5 | 3 | `dssys_2.c` and `dssys_2_3.c` GCC 2.7.2 with GNU as; DsInit and DsReset (`dssys_2_2.c`) need GCC 2.8.1 with `-fno-schedule-insns2`: they clear two tables through a base register, which GCC 2.7.2 folds into absolute addresses (lev 43) |
| LIBDS DSREAD | 2 | 2 | `dsread.c` GCC 2.7.2 (`DS_read_cbsync` has an unfilled return), `dsread_2.c` GCC 2.8.1 (base registers, lev 123 under GCC 2.7.2). Best single set GCC 2.8.1 with GNU as and `-mno-split-addresses`, lev 32 |
| LIBDS DSFILE | 2 | 2 | `dsfile.c` GCC 2.8.1, `dsfile_2.c` GCC 2.7.2. Best single set GCC 2.7.2 with GNU as, lev 45: GCC 2.7.2 gives `DsSearchFile` a 96-byte frame, retail 80 |
| LIBDS DSREADY | 5 | 2 | `dsready.c` GCC 2.8.1: `DsStartReadySystem` stores eight fields through a base register, which GCC 2.7.2 folds into absolute addresses (lev 23, `DsEndReadySystem` 9); `ER_cbsync` in `dsready_2.c` keeps the unfilled `jr ra` delay slot that GCC 2.8.1 always fills, so the object cannot move to GCC 2.8.1 either |
| LIBSPU SPU | 4 | 3 | `spu.c` GCC 2.7.2 with GNU as, `spu_2.c` GCC 2.8.1, `spu_3.c` GCC 2.7.2 with the GNU as 2.8.1 assembler: `_spu_FsetRXXa` needs the old assembler's `remu` expansion (divide before the zero check) and `spu.c` the new assembler's `jal` delay-slot fill with the low half of `la`; no assembler here does both, as ASPSX did. `_spu_t` (`spu_2.c`) keeps a base register that GCC 2.7.2 folds (lev 26) |

Merges after a new C shape for the part that needed other markers (LIBAPI
PAD, LIBPAD PADIF, LIBMATH MULDF3 and LIBPAD PADPORTD are one unit each now):

- LIBAPI PAD (`pad.c`, GCC 2.7.2 with GNU as): `Sys_SetInterruptHandler`
  stores its second handler through a post-incremented pointer. With
  `handlers[1] = ...` cse folds the register-plus-offset address into the
  constant `D_800A34B4+4` (find_best_addr folds every non-register address)
  and the store goes out absolute, which is why the part used GCC 2.8.1.
- LIBDS DSSYS_2 parts 1 to 3 (`dssys_2.c`): `CQ_delete_command` copies
  the read index through a second pointer variable (`state = end - 1;
  *end = *state;`), which GCC 2.7.2 keeps as `-4(v1)`; `end[-1]` folds to an
  absolute address. Its three asm barriers were only needed under GCC 2.8.1.
- LIBPAD PADIF (`padif.c`, GCC 2.7.2 with GNU as and
  `-fno-cse-follow-jumps`): no source change. `CardObj_WriteCommandByte`
  loads `obj->command` again for the call after testing it; GCC 2.7.2 reuses
  the tested value when cse follows the branch into the `else` block, and
  with that option off the whole object matches, `LIBPAD_PADIF_text_26C`
  included.
- LIBMATH MULDF3 (`muldf3.c`, GCC 2.7.2 with GNU as): `_mul_mant_d` only
  failed to build with GCC 2.7.2, whose inline asm has no `+` constraint;
  the two `"+m"` barriers it used are not needed under GCC 2.7.2.
- LIBPAD PADPORTD (`padportd.c`, GCC 2.7.2 with GNU as): `PadInitDirect`
  stores the two output pointers through the `cursor` variable it later
  walks the actuator maps with (`obj[1].output_30 = ...` folds into an
  absolute address under GCC 2.7.2), and its final memory barrier is not
  needed. `MemCard_DmaCompleteCallback` keeps its barriers, now in GCC 2.7.2
  syntax: the callback result launder moved to the end of the loop, the
  cleared test goes through a flag (`cleared = callbackResult == 0`) so
  the store stays `sw zero`, and the I/O pointer and index are loaded in
  one barrier.
- LIBCD SYS parts 2 and 3 (`sys_2.c`, GCC 2.7.2 with GNU as): `CdControl`
  and `CdControlF` copy the incoming command through their retry counter
  before the counter gets its start value (`tries = cmd; cmd_reg = tries;
  tries = 3;`), which gives the retail register order without
  `-fno-schedule-insns`; `CdIntToPos` and `CdPosToInt` match under the
  same set.
- LIBDS DSREADY parts 2 to 4 (`dsready_2.c`, GCC 2.7.2 with GNU as):
  `LIBDS_DSREADY_text_3D8` reads the saved callbacks through a second
  pointer (`active = state + 1`) and wraps its second early return in a
  `do { } while (0)`. The loop ends cse's jump-following path, so GCC 2.7.2
  no longer knows the state pointer is a constant and keeps it in `$s0` as
  retail does; without it every field access folds into an absolute
  address. `ER_retry` no longer needs its `$3` register pin.
- LIBSPU SPU parts 4 and 5, now part 3 (`spu_3.c`, GNU as 2.8.1): `_spu_Fw1ts` counts
  its volatile loop index with `i = i + 1`; `i++` reads the volatile twice
  under GCC 2.7.2.

### AKAO driver (src/main/akao)

The AKAO units have no offset macros, byte arithmetic or file-scope
`extern`s: driver-wide state is declared once in `pe1/akao/globals.h`, the
voice-mask SPU registers in `pe1/akao/spu_registers.h`, and the sequencer
and transfer units carry no pins, barriers or gotos. C shapes that only
exist for the bytes, and why:

- `Spu_InitVoiceState` writes the request, volume and slide fields of the
  two banks behind `g_AkaoCurTrack` through word views
  (`*(int *)&bank[0].volume`). GCC 2.7 assumes a store to a structure field
  through a pointer cannot alias a fixed scalar, so with plain field stores
  the scheduler hoists the `g_AkaoVoiceUpdateFlags` read-modify-write and the
  `D_800BCD54`..`g_AkaoSfxTickAccumulator` stores across them. Retail keeps the source
  order, which only non-structure stores give. Declaring the scalars as a
  record instead fixes the stores but not the read-modify-write, which a
  record view turns into a register-held address. `Akao_UpdateVoiceMask`
  writes `key_off_request_mask` the same way, and `Spu_StepReverbLoad`
  reads the bank cursor through the one-field `AkaoBankCursor` record for the
  opposite reason: its reload after a bank store must stay.
- `Akao_SetBgmVolumeFade` reads `g_AkaoBgmHandle[0]` twice by index. A
  fixed-symbol load is not reused across the stack store of the fifth
  argument, which gives retail's reload without a volatile slot pointer.
- `g_AkaoMessageQueueLock`, the message-queue lock the timer interrupt tests, and
  `D_8009D2E0`, the busy flag `Spu_PollOrWaitSeqBusy` spins on, are
  `volatile`: `Spu_InitVoiceState` clears the lock and reads it back before
  copying it to the queue words, and only a volatile object keeps that
  reload.
- `Spu_UploadSampleBlock` walks the header words with `*header++` on its
  parameter. A separate cursor lets CSE fold every word into an offset from
  the untouched parameter, while retail advances the pointer.
- `Spu_RestoreVoiceFromStereoSlot` reads the stereo slot's right volume
  into a halfword before the `update_flags` read-modify-write and stores it
  after. Copying the field directly reorders the loads and stores around the
  flag update and loads the index from `$s1` instead of the argument
  register (lev 12). The 0x1FF93 that retail keeps in `$a3` is the mask
  register, not a fourth argument.
- `Spu_TickVoiceEnvelopes` computes the pitch LFO depth inside both
  branches of the speed test, into a `u16`. Retail shifts the depth before
  loading `pitch_lfo_phase`: a depth assigned once after the branches is a
  single-set pseudo that sched1 places late, and a `u32` depth assigned in
  both branches is allocated to `$v1`.
- `Akao_LookupPitchPeriod` assigns the octave copy again inside
  `if (detune)`; without it CSE substitutes the octave for the copy and the
  two values share a register. `SeqOp_LoadInstrumentFromIndex` and
  `Akao_LookupPitchPeriod` keep integer table addresses, because retail
  reuses that register for the release mode and the period (lev 15-24 with
  `AkaoInstrument` pointers).
- The `Spu_*AllVoice*`/`Spu_*VoiceMasked` loops of the voice-bank unit walk
  a cursor biased to one field of the channel record (`voice - 0xF4`), as
  retail does; a plain `AkaoTrack *` walk addresses the fields from the
  record start (lev 6-12 per function). The voice loops that bias their
  cursor to `flags`, `pan_duration` or `adsr_release_rate` reach the record
  back with `AKAO_TRACK_OF(cursor, member)` (`pe1/akao/track.h`) and step
  by `sizeof(AkaoTrack) / sizeof(*cursor)`.
- The stream uploads copy the relocated instrument records into their
  cache slot through `AkaoInstrumentAddress` (`cache.word = offset +
  cache.word`), which keeps retail's offset-first `addu` without byte
  casts. The playback-bank messages take their `AkaoQueueEntry`, whose
  `arg0.sequence` is the sequence data `Akao_UpdateVoiceMask` rebases.

Crutches still in place (lev is the only metric; each was retried with the
permuter and by hand):

- `Akao_StepNoteSequencer` and `Spu_UploadStreamBlockA` (4 barriers each):
  retail allocates the header cursor before the upload size (s0, s1).
  Global-alloc priority is refs * floor_log2(refs) / live length; without the
  barriers the size has 22 refs over 84 insns against the cursor's 18 over
  83, and the barriers add the cursor references that flip the order. A
  separate copy-loop counter removes the cursor barriers but lands the counter
  in a2 (lev 3); reusing the size as the counter flips priority back (lev 31).
  Clamping the continuation upload into its own block-scope size takes the
  size's continuation references out (`Spu_UploadStreamBlockA` then differs
  only by the s0/s1 swap: cursor 18 refs over 42 insns, size 16 over 31,
  priority 1.71 against 2.06). One size reference fewer flips the order, but
  every plain C shape that drops one (the clamp as `if`/`else`, the counter
  in its own variable, the size advance inlined) changes the clamp branch or
  the counter register; permuter runs found only invalid or duplicated-code
  shapes.
- `Akao_ProcessVoiceQueue` (pin on `bit`, 1 barrier): `secondary_new` must be
  allocated before `excluded`; its extra references come only from a copy that
  CSE deletes unless `bit` is a hard register (lev 24, lev 11 with a
  `do { } while (0)` wrapper, which is no better a crutch).
- `Akao_StepSequencerVoice` (pin on `all_voices`, 1 barrier): retail builds
  0xFFFFFF once in a0 and stores it from a0; cse1 rewrites the argument copy
  to the constant and builds it twice (lev 3-15). Passing the stored field
  back (`func_80089960(pend->allocated_voice_mask)`) reloads it (lev 6), and
  the assignment as the argument builds it twice (lev 4).
- `Seq_TrackPitchSetup` (2 pins): retail computes the pitch delta in place in
  a2 (sll, subu, div, mflo), which gives it more weight than `track`; without
  the pins the subtraction goes through v0 and the registers swap (lev 20).
  Dividing in place (`delta /= duration`, the `Seq_SlideTrackPitch` shape)
  gives lev 29.
- `Akao_UpdateVoiceMask` (3 pins, 4 barriers): without the v0 pins loop.c
  also hoists the constant 2 (its threshold of 52 equals the loop's 52
  insns); with literals the hoist set matches but the bank copy and
  `voice_mask` scheduling move. Best crutch-free version is lev 36.
- `Seq_StartNestedStreams` (1 goto): retail tests `remaining` again after
  the exhausted handler, which happens only when the handler starts at a
  label CSE enters without knowing `remaining`; every goto-free shape lets
  CSE drop that test (lev 69).

### Tables, assets, process manager and sys (src/main/{table,asset,pm,sys})

These units have no file-scope `extern`s; their data lives in
`pe1/resident_tables.h`, `pe1/pm.h`, `pe1/asset_table.h`, `pe1/boot_services.h` and
the other owner headers. C shapes that only exist for the bytes:

- The resident data pack at 0x800A8028 (`g_ResidentDataPack`) is read
  directly by most lookups, which lets CSE address the pack base relative to
  the slot word as retail does, and through a local `ResidentDataPack *` by
  the text-table lookups, which keeps the slot load and the pack base
  independent as retail does.
- The process-manager slot helpers take the secondary index into its own
  `int idx = slot - 11`; indexing with `slot - 11` directly converts it to
  the unsigned size type first and loop strength reduction no longer
  produces the retail per-bank offset counters. `Pm_StopUpperHalf` passes
  `i + 11` so the slot number is its own induction variable.
- The keyed asset lookups store the payload in `found` before returning it
  (retail allocates `found` above the loop index), and each keeps an unused
  `char unused[1]` local because retail reserves an 8-byte frame it never
  touches.
- `Asset_SearchByKeyType` keeps its masks in locals so loop.c hoists all
  four into saved registers, as retail does; literals in the return
  expression stay in the exit block.

Crutches still in place (lev is the only metric):

- `Scene_LoadRoomAssets` (1 goto): the PE.IMG retry jumps back over the
  sector read. Any loop form (do/while, for with break) is seen by loop.c,
  which hoists the table address and the -1 constant out of the retry path;
  retail rebuilds them on every retry (lev 50).
- `Pm_Exec` (1 memory barrier): retail loads `entry->state` again after the
  active-state test; without the barrier reorg deletes that load as
  redundant while stealing the branch targets' delay slots (lev 26 across
  plain, switch, inline-predicate and wrapped forms).
- `Asset_LoadTimTextures` (2 gotos): the state machine restarts its
  `switch` after states 0 and 0x35; a `for (;;)` loop around the switch lets
  loop.c move code out of the cases (lev 81). The two volatile bank-pointer
  views are gone: the bank pointers are stored and reloaded through
  `g_PmTables` (`pe1/pm.h`), a record view of the three adjacent dispatch
  words at 0x800942E0. A record member at a fixed address may alias the
  slot stores, so loop.c keeps the reload in the loop and takes the address
  into a register, which is retail's code exactly.
- `Asset_ResetGameState` (3 pins, 3 launders, byte offsets off `base`): the
  eight ordering barriers were redundant and are gone. Each remaining
  launder makes a -1 or the loop counter look like a fresh value after a
  store so it is not merged with an earlier constant (lev 11, 2 and 14 when
  removed one at a time); the `$3`/`$5` pins keep the fill cursor and loop
  counter in retail's registers (lev 3, 76, 77). A field-by-field rewrite
  through a `Pe1GameState *` gives retail's absolute-then-relative
  addressing and loops, but sched1 launches the shared -1 byte constant
  just before its first store while retail loads it early into v1, which
  shifts every later register (best lev 92).
- `Boot_InitMemoryLayout` (2 pins, 4 pointer-integer casts, volatile
  `g_SceneBankAssetSource`..`D_800B0E68` views): without the pins the buffer cursors and
  the 0x8000/0x48000 constants trade registers in every declaration order
  (lev 44), and without volatile the scheduler interleaves the game-state
  stores with the render buffer stores differently.

### Render (src/main/render)

The render units have no file-scope `extern`s, raw offset dereferences or
alias declarations: camera and scroll state is declared in
`pe1/geom_state.h`, `pe1/render_camera.h` and `pe1/render_tint.h`, the
object and wireframe data in `pe1/render_object.h` and
`pe1/render_animation_frame.h`, and the GTE register transfers are PSY-Q
style macros in `pe1/gte.h`. C shapes that only exist for the bytes:

- Every 8 unused bytes in a render frame came from a top-tested loop with a
  variable bound, or a guarded `i = 0; if (i < count) do ... while` loop:
  combine folds the first compare into the branch and the orphaned compare
  pseudo keeps its stack slot. `Render_DrawTextDigit`, `Font_FindCodeSlot`
  and `Render_ResetAllEntries` lost their stack pads this way.
- Offsets from the geometry header (`g_GeomState + g_GeomState->field`) go
  through the `GeomStateAddress` word view (`GEOM_STATE_OFFSET`): retail
  loads the header pointer twice and adds the offset as an integer.
- `Render_StepFade` and `Render_ApplyScreenTint` advance the primitive
  pointer inside the loop body; in the `for` header the step is scheduled
  above the colour stores (lev 2).
- The wireframe depth and screen-XY buffers are one record
  (`g_WireScratch`), so the line packet's XY stores and the depth loads are
  both structure accesses and keep retail's order without volatile.

Crutches still in place (lev is the only metric):

- `Render_SetViewport` (6 pins, 5 barriers): removing any single one costs
  lev 2-46; the best crutch-free rewrite is lev 65.
- `Render_UpdateScrollPosition` and `Render_PrepareFrame` (volatile register
  screen-centre input, view-matrix pin and slot barrier each): retail has 8
  frame bytes nothing touches. The volatile register is the only form found
  that keeps them; without it the frame shrinks (lev 6 each), a plain pin
  keeps the registers but not the frame (lev 2), and a 12-byte projection
  buffer keeps the frame but swaps the offset registers (lev 4).
- `Render_DrawSpriteEntry` (21 pins, 2 barriers, 6 pointer casts, 1 byte
  sum): each pin alone is worth lev 2-69, the natural rewrite is lev 120+.
- `Render_IncrementCounter` (2 pins): the counter word sits in .text right
  after the routine and it works in a0/a1, so the original is hand-written
  assembler.
- Font selection: `Render_StepFontLoad`, `Render_FindFontGlyphSlot` and
  `Menu_FindSelectedEquipSlotItem` store the selected slot and reload it
  through the other symbol for that byte; without the memory barrier the
  reload is scheduled first (lev 25, 10, 11). `Render_StepFontLoad` keeps an
  identity constraint on the digit (lev 19) and `Render_SetFontGlyphByCode`
  one on the stored slot (lev 14). `Render_LoadFontGlyphAlt` reads the
  glyph through `groups.codes[slots.indices[...]]` but reloads the stored
  slot as `*(u8 *)&g_FontSelectionSelectedSlot`: a plain read is CSE'd with the store
  (lev 35) and `g_FontSelectionState.selected` is hoisted above the store
  because it is another symbol (lev 36).
- `Render_DrawSpriteEntry` adds the ordering-table offset and the position
  array's byte offset through the `RenderLinkAddress` pointer/word union
  (`ot.word = ot_index + ordering.word`): `ordering + ot_index` swaps the
  `addu` operands (lev 2), and the position array lies `pos_ptr` bytes from
  the entry, an offset read from the data.
- The font glyph loads (`Render_LoadFontGlyph`, `Render_LoadFontGlyphAlt`)
  restart a failed poll with a tail call (`return Render_LoadFontGlyph(code);`).
  GCC 2.7 expands a self call in `return` position as a jump back to the
  start of the body, and that jump carries no loop notes, so loop.c hoists
  nothing and the table address and -1 are rebuilt on every retry as in
  retail. The test needs an empty frame at the `return` (no stack locals or
  inlined callee locals declared before it), parameter types equal to the
  argument types after promotion (an `int` code, not `u8`), and a non-void
  return type (a void function never reaches the tail-recursion test).
  `Render_LoadFontGlyphAlt` therefore returns `int`, as its map-selection
  caller already read it.
- `Render_InitDisplayLists` (2 gotos): its retries restart in the middle of
  the function. Moving each read into a `static inline` helper that
  tail-calls itself removes the gotos and rebuilds the addresses (the helper
  never gets an out-of-line copy), but the poll status becomes a
  helper-local pseudo; retail's function-wide `status` conflicts with `$v0`
  in the later `switch`, so retail copies each poll result to `$v1` and the
  helper form tests `$v0` directly (lev 4 plus branch offsets, 58 in all).
  `CdRom_LoadInitialImage` cannot use the helper: the inlined
  `CdRom_PollReady` puts a stack local in front of the `return`, and
  `CdRom_FindNextDataSector` restarts after its setup, so the helper's
  parameter copies stay as moves at the reentry label (lev 30).
- `Asset_OpenPeImage` (2 barriers): retail leaves the delay slot of the second and
  third `found == 0` tests empty; without the barriers reorg fills it with
  the -1 constant (lev 5-8).
- `Render_DrawWireframeModel` (1 goto): the floor-edge search shares the far
  vertex assignment between two arms; written twice, the extra reference
  swaps the screen-XY registers (lev 14). Its file comment lists the
  uncounted register-allocation shapes it still needs.
- `Render_DrawActorShadow` (1 volatile store): retail copies the part matrix
  word by word and keeps the translation store that the next store
  overwrites; a structure copy keeps the store but uses GCC's four-register
  block move (lev 22).
- `Render_DrawObjectVariant` was left as is (out of scope for this pass).

### Menu (src/main/menu)

The menu units have no file-scope `extern`s and no local extern lists except
the two alias views listed below. Menu-wide state lives in
`pe1/menu_state.h`, the bonus point and level-up panels in
`pe1/menu_bonus_stats.h`, dialogs and save slots in `pe1/menu_dialog.h`,
equipment screens in `pe1/menu_equipment.h`, list adapters and row drawers
in `pe1/menu_item_rows.h`, and the event queue in `pe1/menu_queue.h`. C
shapes that only exist for the bytes:

- MASPSX fills a load delay before a `$gp` store only when it knows the
  stored symbol is small data, which it learns from COMMON definitions in
  the unit (`--use-comm-section`). Units that store a loaded value into an
  extern word therefore define it tentatively (`int g_MenuEquipSwapItem;`,
  `g_MenuInventoryViewMode`, `g_MenuPendingTotalExp`,
  `g_BonusPointDisplayValue`, `g_BonusPointSpendCurrentValue`) instead of an
  explicit NOP. `menu/equip_screen.c` uses `MASPSX_FORCE_G0` with COMMON
  definitions for every `$gp` word so `&D_8009CF90` stays an absolute `la`.
- Retail inlines `Queue_Enqueue`, `Queue_DequeueByMask` and
  `MenuWidget_AllocNode` into their callers; the units declare them `inline`
  (non-static, so the out-of-line copies remain) and call them.
- A node variable assigned once places its copy of the call result late
  (sched1 launch priority), one assigned several times keeps it right after
  the call: `Menu_OpenRenameScreen` reuses one parent variable, the equip
  confirm dialog takes its own `dialog` variable, `Menu_StepSkillScreen`
  stores the second lookup back into `panel`.
- Two-line and item dialogs size their panel with `MAX(100, MAX(w0, w1))`
  of `Draw_MeasureTextWidth` calls: the macro's repeated calls and the
  cross-jumped tail are what retail does.
- `Menu_StepColorSelect` is an `else if` chain with one `return 1` at the
  end: both channel arms end in the same blend tail, and with the jumps to
  one return label cross-jumping keeps the second arm's copy as retail does
  (separate `return 1` statements keep the first copy). The channel shift is
  `index << 3`: `index * 8` expands through a temporary that cse later
  swaps with the variable, leaving one clamp with a copy of the shift in s3.
- Shared tails come from cross-jumping, and which copy survives depends on
  how the copies leave: copies that jump to one join (an `else if` chain
  with a single `return` at the end, `break` out of a `switch`) collapse
  into the last copy; copies that each end in `return` collapse into the
  first. `Menu_SpendBonusPoints` uses both: the decrement cases end in their
  own move/error sounds and `break`, so the success tail stays in the last
  case and the error call after it, while the increment arm keeps its own
  move sound with an early `return` and falls through to one error call.
  This replaced its seven `goto`s, the `$2` refund pin and the four
  barriers that kept the decrement cases apart.
- Small decision trees are `switch` statements with an empty `case 1`
  (`Menu_ContextPick`) or separate identical cases (`Menu_OpenInventoryOrSwapView`).
- `Menu_AlignEquipPanels` computes the x shift from one conditional
  expression; the separate `if` chain gives the loads other registers.
- The cursor stack is walked as `int` word pairs, except in
  `MenuWidget_DrawListRow`/`RowPopCursor`, where the `DrawTextCursorPair`
  structure view lets the scheduler move the global loads above the stores.

Crutches still in place (lev is the only metric):

- `Menu_StepEquipConfirm` (4 pins, 1 barrier, 7 gotos): retail places the
  confirm tail right after the first arm and the reserved/dialog arms jump
  back into it, an order only the label layout gives (if/else chain lev 71);
  each remaining pin alone costs lev 18-43.
- `Menu_StepItemSelectScreen` (3 pins, 5 barriers, 2 gotos): retail returns
  through the cell-index variable in s0 (`li s0,1` then `move v0,s0`); plain
  returns fold to `li v0,1` (lev 44). Case 8 needs its three argument
  registers loaded before the `D_8009CF30` store (lev 2 without the pins).
- `Menu_InventoryInputHandler` (1 pin, 3 barriers): same s3 return value;
  early returns lev 19, a single-exit chain lev 45.
- `Menu_InventoryPageNavHandler` (3 pins, 1 barrier): the conditional shift
  that matches `Menu_AlignEquipPanels` gives the shift a higher global-alloc
  priority than the target node here (s1/s2 swap, lev 12).
- `Menu_DrawItemDetailPanel` (2 pins): retail computes dx right after its
  two loads; unpinned, dx is scheduled after the tail-count chain (lev 5).
- `Menu_DrawStatsList` (2 pins): with either pin gone loop.c
  strength-reduces the multiplier index into a sixth saved pointer (lev 20-25).
- `Menu_StepItemDetailPanel2` (1 pin): retail reloads the label buffer
  address before each of three calls; unpinned, CSE keeps it in a saved
  register and label/option node swap (lev 18).
- `Menu_StepSaveLoadScreen` (1 pin): the transfer callback and
  `Menu_InventoryInputHandler` addresses both live in saved registers and
  swap s0/s1 unpinned (lev 18).
- `Menu_StepSkillList` (1 pin): retail builds the 1 of `1 << type` in v1;
  every plain form folds it into a0 (lev 2).
- `Menu_StepSaveSelect` (1 pin): the dialog mode lives in a1 and is passed on
  to `Save_SetTitleStyleFlag`; unpinned, slot and mode swap a0/a1 (lev 9).
- `Menu_OpenInventoryScreen` (1 pin): retail loads 9 into a0 once before the
  branch for `MenuWidget_InitPool(9)` and `Inv_SetActiveList(9, 0)`; the
  propagated constant leaves a second `li a0,9` (lev 1; an unpinned local
  for both calls is also lev 1: cse substitutes the constant on the
  fall-through arm and the delay slot of `MenuWidget_InitPool` gets the
  second `li` instead of a `nop`).
- `Menu_DrawActiveListSlot` (1 pin): retail copies the item ID into a1 before
  the range tests (lev 12 without).
- `Draw_InitBuffers` (1 pin, 1 barrier): retail stores the screen
  rectangles relative to s0 (the first frame's packet arena field); constant
  addresses fold to absolute stores.
- `Draw_SetPrimCallback` (1 memory barrier, 1 volatile read): retail
  re-reads y_limit and the other fields after storing them.
- `Aya_SetTotalExp` (1 barrier): retail keeps the save base in s2 for the
  total and the cleared pool; without the launder the constant base folds
  into absolute loads (lev 6). The bonus pool is read and written through
  the `g_AyaBonusPoolRecord` record view (the unit is forced-G0).
- `Menu_MainUpdate` (1 pointer-integer cast): the m2c variables that carry
  the battle entity pointers also carry colours. The dead-player motion
  clear and the tail position pointer use register unions of the colour word
  and the entity pointer, which keep one pseudo and so the allocation. The
  reset actor pointer keeps its cast: the same union is lev 8, because a
  store to a union member is expanded through a fresh pseudo and copied, so
  the actor load is no longer tied to the mask register and is scheduled
  early (a separate single-set pointer is lev 7-8). The halfword position
  reads go through `Pe1Fixed_IntegerHalf` (see the recipes below). The draw
  slot reloads come from the `g_ActiveDrawSlotRecord` record view and the
  BGM handle reload from an array element read (see the recipes below).
- `Menu_StepItemGrid2` (two asm-labelled views of `D_800A19C0` in
  `pe1/Menu_MemCardProgress_externs.h`): retail rebuilds the text buffer
  address for each `Draw_MeasureTextWidth` call; with one symbol CSE keeps
  it in a saved register (the `MAX`-style repeated call form is lev 42:
  GCC compares SYMBOL_REFs by name, so only a second name is a second
  constant).
- `Battle_SetWindowBoundsByMode` (volatile window X and bound pointer): plain or
  multi-set forms keep the order but swap v0/v1 (lev 5).
- `Menu_DrawTextboxEntries` (pins, barriers, 6 gotos, stack reserve),
  `Render_SetupColorTable` (5 pins, 3 barriers) and `Render_SetupFogLayer`
  (2 pins, 1 barrier): the glyph spacing offset must stay in s7 across the
  packet calls (lev 127 with structure indexing), a plain digit loop is
  lev 95. The byte offsets into the draw buffers, ordering tables and glyph
  spacing table go through block-scope `TextboxTableAddress` unions
  (`pe1/textbox.h`). Two fog packets keep their `(int)` sums: the union
  copy of the `$11` sprite base or of the laundered payload costs a
  register (lev 6). The section and delay loops of cases 6 and 7 keep their
  backward `goto`s: as loops (with a block-scope word so the exit test
  stays at the top) loop.c hoists `(sectionIndex + 1) << 16` and the
  spilled section index reloads (lev 69-71). The 0x38 stack reserve is a
  stack slot retail gave a dead address pseudo: indexing
  `other_buffers[slot + 1]` directly recreates such a slot, but above the
  spilled locals instead of at 0x38 (lev 38).

### Field effect engine (src/main/fieldfx) remaining crutches

Swept 2026-10-11 and 2026-10-12. Recipes that removed crutches without
changing bytes:

- preset initialisers written as plain per-record constant stores (record
  order, fields scale, rotation, cell, clut, depth, rgb); `actor->pos_x >> 16`
  instead of `Pe1Fixed_Integer()` where retail has `lh`; guarded do-while plus
  padding or `asm(&frame)` replaced by `for` loops (8-byte loop frame slot);
  `void` return where retail fills the epilogue delay slot; deleting the
  `-fno-schedule-insns` markers of CollisionQueries_2 and BurstEffect_2.
- GTE macro operands are the objects themselves, not pinned pointers:
  `gte_ldrotmatrix(g_ViewMatrixSlot.value)`, `gte_ldv3(g_FieldFxCellQuad, &g_FieldFxCellQuad[1],
  &g_FieldFxCellQuad[2])`, `gte_ldv0(&g_FieldFxCellQuad[3])`, `gte_stsxy3(&p->x0, &p->x1, &p->x2)`,
  `gte_stszotz(&depth)`. A constant or memory operand of an `"r"` input is
  loaded by reload, and reload takes its spill registers round robin
  (`last_spill_reg`); the number of spill registers is the largest number one
  insn needs at once, so the three-operand `gte_ldv3` is what gives the
  t1, t2, t3 rotation retail has in the sprite renderers. Separate
  `gte_lwc2_*` statements or one-operand loads leave a single spill register.
- The composed glow layers are two (`fieldfx/glow_layers.inc`) PSY-Q
  `gte_CompMatrix(&matrix, &rotation, &matrix)` calls; the floor-triangle
  tests are the PSY-Q outer product (`gte_ldopv1_psyq`, `gte_ldopv2`, OP,
  `gte_stlvnl`) with the first operand read through `g_FieldFxTriScratch` and one
  `FieldTriScratch *` local per later product.
- A byte field incremented as `age++` is a QImode subreg set, which sched1
  does not give launch priority, so the increment is hoisted to the top of the
  block; `age = p->age; p->age = age + 1;` through an `int` makes it a
  single-set pseudo and sched1 places it where retail does (the four glow
  particle updaters). The same holds in reverse for `FieldFx_SetSpriteCellSize`: `u8`
  cell-size temporaries are not launched, so width-1/height-1 are computed
  first as in retail.
- The particle updaters' `move a0,a2` is a second pointer to the particle
  (`FieldFxMovingParticle *particle = anim`) used only for the lifetime
  countdown after the bounce test.
- `FieldFx_UpdateStripCollision` walks the strip nodes beside a `FieldStripNodeTail` view
  (the node seen from its brightness field, `pe1/field_textured_chain_node.h`):
  `body[-1].nextCellStep`, `body[1].edgeA` replace the raw byte offsets.
- The field engine object passes (`fieldfx/object_script.c`) keep the slot
  table and script state in typed globals (`FieldFxSlot *`,
  `FieldFxState *`); the slot address the two dispatch loops add as an
  integer, index first, is a `FieldFxSlotAddress` pointer/word union.
- `fieldfx/glow.c` .. `fieldfx/burst.c` are the eight effect
  module classes of `pe1/field_effect_modules.h`; their scripts, handler lists,
  slot sizes and tables are declared there under module names instead of the
  per-unit extern headers.

- A copy of a parameter that retail keeps in its own register survives cse
  when the parameter itself is reassigned afterwards: `FieldFx_AcquireCallbackSlot` reads
  the list header through two copies taken before the `list` parameter
  advances to the entries (`list += 0xC`), which gives retail's `v0`/`t0`
  copies without pins or barriers. The `collision_step` post-increment
  (`step++ == 0x64`) loads the halfword once with `lhu` and sign-extends it
  for the test, as retail does, without a volatile view.

What is left (best crutch-free lev against retail):

- `fieldfx/angle.c` (5 pins): `FieldFx_TurnToward` lev 15 and
  `FieldFx_AngleDistance` lev 6. Retail keeps the step and the turn delta in separate
  registers (`move v1,a2` then `negu v1,a2`): with `delta = step` followed by
  `delta = -step` cse makes the longer-lived delta the class head and rewrites
  `-step` as `-delta`, so step dies at the copy and global alloc gives delta
  step's register (a s16 delta, if/else and ternary forms tried). The same
  copy preference ties `FieldFx_AngleDistance`'s distance to the difference in `a0`.
  Declaring the turn delta `unsigned short` breaks the cse equivalence (the
  copy is a truncation), so delta keeps its own register: lev 8, the rest
  being the zero extension before the final add and the difference computed
  into `v0` then copied (`short` lev 9). The distance helper stays at lev 6
  for `s16`/`int` distances, if/else, ternary and in-place `lhs -= rhs`.
- `fieldfx/collision.c` `FieldFx_UpdateStripCollision` (1 pin on the node pointer):
  lev 9 without the pin. Global alloc ranks the loop index (9 refs over 45
  insns) above the node pointer (11 over 59); retail has node, index, strip in
  s1, s2, s3. Dropping the index's reuse as the return value lets the strip
  outrank the index (lev 13); starting the index inside the `if` gives the
  node s1 but swaps strip and index (lev 17-21).
- `fieldfx/geometry.c`: `FieldFx_DrawRotatedQuad` keeps one empty constraint on
  `rotation` and `length` (lev 71 without it: `rotation` is spilled to the
  stack instead of living in s8 across the packet setup calls).
  `FieldFx_DrawPulledSprite` keeps the entry memory barrier, the `$6` clobber and the
  volatile stack parameters (lev 14 without them: sched2 interleaves the
  stack-argument loads with the register saves, retail loads them after the
  last save).
- `fieldfx/glow.c` `FourLayerGlow_DrawFlareLayers` (5 pins, 1 barrier): lev 50
  as four `gte_CompMatrix` layers. Local alloc ranks the composed translation
  source (`&rotation.t`) above the second-column addresses and leaves the
  matrix second column without a register; retail spills the translation
  source instead (stack slot 192).
- `fieldfx/geometry.c` is excluded from this sweep (owned
  elsewhere); its three `_externs` headers stay with it.

### Open declaration conflicts

`python3 tools/scripts/decl_conflicts.py --count` lists symbols declared more
than once with different types (629 at the start of the header unification,
232 after it, 99 after the battle pass, 65 after the small-data pass, 12
after the actor and overlay view pass, 1 after the scalar view pass, 0 after
`GetClut` took its header declaration; 29 again after the cast removal
rounds exposed the types the casts hid, 0 after the pass that gave each
symbol its retail type in one shared header). The scanner drops file-scope
body macros such as `ROOMLIB_CLOSE_TARGET(name)`, which end without a
semicolon and would otherwise be glued to the next declaration.

How the earlier groups were closed, for the next symbols of the same kind:

- Small data versus absolute access is solved per unit, not per symbol: a
  `-G8` unit that reads a small word absolutely while its own small data is
  `$gp`-relative takes `/* MASPSX_FORCE_G0: 1 */` with
  `--use-comm-section`, and tentative (COMMON) definitions of the words it
  does reach through `$gp`. MASPSX then emits `$gp` loads only for that
  COMMON metadata and every extern stays absolute, whatever its declared
  size. A unit whose `$gp` words are records read at a field offset also
  takes ASPSX 2.70 (a marker then, the default now), which lets MASPSX keep those offset
  accesses `$gp`-relative (`Geo_ActorFloorResolution`, `Menu_TextboxDraw`).
  A unit that also needs cc1 to treat a small word as expensive (address
  kept in a register) gets `-G0` for cc1 instead; that is why
  `Draw_InitPipeline` is its own unit, ahead of the `-G8` frame loop. This
  retired the array, record and aliased views of the battle, task, boot,
  menu and field words, the actor list head (`D_8009D20C`,
  `g_FieldActorListHead`), the player (`D_8009D254` as a `BattleEntity`
  pointer, `g_PlayerEntity` as a `FieldActor` pointer), `g_ActiveActor`,
  `g_ActiveDrawSlot`/`D_8009CDDC`, `g_GameStateFlags`/`D_8009D1A0`,
  `D_8009D1F4`, `D_8009D2E8` and the task script cursor `D_8009CE00`.
  The symbol alias pass applied it to `Battle_EscapeAndPhaseInitFlow`,
  `Battle_DrawStatusPanel`, `Battle_InitEntityColors`,
  `Inv_InitMaxLevelInventory`, the three `-G8` capacity units and the `-G4`
  `Inv_CheckFreeSlotCapacity`, which retired the `battle_turn.h`,
  `battle_status.h` and `inventory_slots.h` views and the per-byte HUD color
  views. Where retail reloads a word after a field store, the plain scalar is
  not enough: GCC 2.7 lets a store to a structure field skip a fixed scalar,
  so `D_8009D29C` stays a one-element array and the `Combatant` flag store in
  `Battle_InitEntityColors` goes through a pointer to the field (an
  indirect store is not a structure field store). A field store that
  `g_GameState` and `D_800B0CEC` already describe is written as that field.
- Pointee views of one pointer (the room and scene records of the player,
  the actor list, the matrix and projection slots) became casts at the use
  to the view each unit reads, or the `FieldActor` field when it has the
  same offset and type; the generated code does not change.
- Element views of one table became a union element type
  (`GteSineWord` for the sine table `rcossin_tbl`) or the record the data is
  (`FieldGlowSprite` for the scene_e08 sprites, `DISPENV`/`DRAWENV` for
  `D_800BCE80`/`D_800BCDC8`, `AkaoTickTrack` for `D_800B8AC0`,
  `FontGlyphLoadState` for `D_800B0DD8`). Where a function needs the old
  array arithmetic, it casts the table at the use.
- Per-overlay data declared differently by two headers of one overlay got
  one owner header in the overlay directory (`room_m350_data.h`,
  `room_m273_boss_state.h`, `scene_e22_ember_data.h`,
  `menu_memcard_offset.h`).
- `BattleCmd_CommitAmmoAndUpdate` is called with the queued action and
  without it, as retail does, so `battle_cmd.h` declares it without a
  prototype.
- Scalar versus record access of one word: cc1 reads a scalar `extern`
  straight from its symbol, but any record, union, array or cast view goes
  through `memory_address`, which puts the symbol's address in a register
  when it is not small data, and CSE keeps it there for a read-modify-write
  or a reload. A record declaration and a record view cast from the scalar
  (`(*(T *)&sym)`) produce the same code, so the word is declared as the
  scalar and the record users read it through a view: `D_800BCF88` is the
  scroll state's flags word (`volatile int`, the grab effects' absolute
  read-modify-write) and `g_GeomScroll` the record view; `D_800B0CD8` is
  `g_GameState.flags` as a `u32` (the frame loop, process manager and CD
  boot read), while every record and array user reads `g_GameState`;
  `D_800B0CE6` is `g_GameState.story_day_flags` as a `u8`. The opposite
  case, a unit that needs the scalar to read like a register-held record,
  takes cc1 `-G0` (`Battle_EncounterInitializationFlow`); a unit that needs
  a record or volatile cast of a small word to read absolutely takes cc1
  `-G8`, which makes the small symbol a cheap address (`FxCommon_*`
  with the projection slot record `g_ProjectionSlot`,
  `RoomEffect_ConfiguredPulseCallback` with `*(volatile u16 *)&g_FieldFxTpageSlot2`).
  The cc1 `-G8` trick needs a unit without small data definitions of its
  own: in `room_lib/grab_attack.c` its `.sdata` is discarded by the overlay
  link.
- The texture slot words `g_FieldFxTpageSlot2`/`g_FieldFxTpageSlot3` are declared once as
  `u16` entries of the slot table (`field_texture_tables.h`); the one-field
  record views became `TEXTURE_SLOT_RECORD`, the array views index
  `g_FieldFxTpageSlots`, and the volatile reads were not needed except in the one
  above.
- `g_RoomScriptRecord` is declared once (`room_script_record.h`) by the
  leading halfword every room's record starts with: room_m123 reads it as
  that scalar, room_m005 and room_m023 take its address through their own
  view macros, and `RoomLib_SetTaskVar0` returns its address.
- A unit in a shared overlay directory (room_lib) now counts towards the
  rooms whose splat config links it, so `room_lib/capture_frame_sample.c`
  (m156/m291/m380) and `room_lib/effect_sequence.c` (other rooms) no longer
  collide at `D_80192C34`/`D_80192C38`/`D_80192C3C`.

Still open:

- (Superseded 2026-10-12: both pairs this item listed as unmergeable,
  `Entity_CollisionDatabase`/`Entity_UpdateList` and
  `Render_EntityAndTexturedQuadFlow`/`Render_DrawObjectAndClut`, are now
  one unit each, `field/actor_collision.c` and
  `render/object.c`, under one FORCE_G0 set with COMMON `$gp`
  words; see "Resident translation units".)
- Symbol aliases that `MASPSX_FORCE_G0` cannot remove, because cc1 decides
  them before MASPSX runs:
  - `CdRom_StepReadState` reads the sound table pointer `D_800B0E08` four
    times with no store or call between the check and the use; one symbol
    lets CSE merge those loads, so its three views stay.
  - `Menu_MemCardProgress` (`D_800A19C0`, two views), `Save_StartWriteSlot`
    (`D_8009EE70` in `memcard.h`) and the scene_e20 hover orb
    (`g_FieldFxTpageTable`) rebuild a buffer or table address after a call or a
    store. With one symbol GCC keeps the address in a register (or, in the
    hover orb, an 8-byte spill slot). The text buffer matches only when it
    is declared at 8 bytes or less, which would misstate its 0x40 bytes.
  - `Battle_StepEscapeOrDeath` keeps the read and write views of
    `D_800B0D88` (`D_800B0CEC.flags_9C`): the player's flags load must stay
    after that store, which needs a non-small store address, and writing
    the field through `D_800B0CEC` makes the loop keep its address in `s1`.
- Crutches left in `src/main/field` and `src/main/anim` (2026-10-11 pass).
  Each was removed and rebuilt; the reason is what changed in the bytes:
  - `Anim_DecodeBoneRotationsShort` keeps the `$s2` pin on the angle
    pointer. The bone counter and the pointer both have 10 references, but
    the counter is dead between the translation loop and the bone loop, so
    its live range is two insns shorter. Global allocation then ranks it
    first and gives it `$s2`. Starting the counter first fixes the
    allocation, but sched2 then moves `move s3,zero` above the `lui`. No
    statement order of the first four assignments matches without the pin.
    Storing the Y sample through `angle->y` instead of `angleZ[-1]` gives
    the pointer the extra reference that fixes the allocation, but then the
    store is addressed from `$s2` instead of `$s1` (lev 1); splitting the
    two loop counters moves other registers (lev 26). Assigning the angle
    pointer only after the translation loop fixes the allocation but leaves
    the `lui` and the `angleZ` address (`lui`/`ori`) after the loop (lev 17).
    The frame byte offset is added through the `AnimSampleAddress`
    pointer/word union. Indexing the halfword samples by frame hoists the
    `<<1` below the `angleZ` setup, and `(u8 *)` arithmetic swaps the
    `addu` operands.
  - `Entity_SlideOnWall` keeps the `$s3` pin and both empty asm statements.
    Retail saves `s3` first and copies the entity from `$a0` in the
    `bgtz` delay slot, after both vertex loads. Without the barriers, sched1
    and sched2 put the two `lh` loads last for their load hazard. The
    parameter copy is a fixed head insn before reload, so neither a
    separate `entity = input` copy nor per-branch copies move it.
  - `Render_LoadMeshEntry` adds 479 to the palette start in place before
    copying it to the running maximum. Retail loads the palette byte before
    the frame is set up; computing the maximum as `paletteStart + 479` makes
    the load a single-set pseudo that sched1 sinks next to its use (lev 6).
    The page pointer and `disp_x` are stored through pointers to the fields
    (`pageSlot = &entry->u34.pagePackets`, `dispX = &entry->disp_x`). GCC 2.7
    lets a load of a fixed scalar (the `D_800B1624` reload) pass a structure
    field store, but not an indirect store, so the reload stays after both
    stores as in retail; this replaced the `disp_x` memory barrier and the
    byte-storage view of `u34`.
    `Render_TransformPoint` keeps its three pins and the barrier on the x sum.
    Without the pins, local allocation swaps `$t0`/`$a0` for the entry and
    y. Without the barrier, the unclamped x copy merges with the sum
    (lev 36 with neither; reusing `x` for the clamp bound or comparing
    before the bound copy changes nothing or is worse, lev 36-38). The
    `D_800B1624` pointer stays volatile: a plain pointer lets CSE reuse it
    across the `disp_x` store (lev 21). The
    four bound barriers are gone; loading the bound before the y add gives
    the retail order.
  - `Geo_ActorFloorResolution`: `Entity_FindFloor` adds the region slot
    offset to the table address as integers, through the
    `CollisionRegionTable` address/entries union (no casts). Indexing
    `D_8009CE08.entries` or starting the sum from the table pointer swaps
    `$v0`/`$v1` in both floor modes.
    `Entity_ResolvePosition` keeps the `$v1` pin on its index copy: retail
    copies the index to `$v1` before the plane-table test and masks it in
    each branch. Unpinned, CSE folds the copy into the parameter and both
    branches mask `$a1` (lev 5); reusing the copy for the Z argument in the
    sloped branch keeps it in the flat branch only (lev 1), and a `u16`
    parameter changes nothing. Masking the copy in place keeps the copy but
    in `$a2` (lev 18); reusing the parameter for the X argument still lets
    CSE fold the copy (lev 10).
    `Geo_PointInPoly` keeps the `$t3` pin on the parity flag: unpinned, the
    flag (10 references over 52 insns) ranks below the current Z (11 over
    42) and the two swap `$t3`/`$t4` (lev 10). Only a duplicated toggle adds
    the references that flip them.
    The ramp `goto slide` enters the slide code past the first floor search;
    a flag variable instead keeps a register and a test (lev 61).
- Crutches left in `src/main/memcard`, `src/main/save`, `src/main/entity`
  and `src/main/cdrom` (2026-10-11 pass). `src/main/save` is clean. Each
  site below was removed and rebuilt; lev is the Levenshtein distance over
  instruction words of the best crutch-free form tried:
  - CD and stream retry loops keep one `goto` each
    (`CdRom_LoadInitialImage`, `CdRom_FindNextDataSector`, `CdRom_StepReadState`).
    Retail hoists no invariant out of these loops: the range pointer, the
    `0xFEFFBFFF` mask and the `-1`/`33`/`42` constants are rebuilt on every
    pass. GCC's loop optimizer only runs on syntactic loops, so a `do`/`for`
    form hoists them into `$s` registers (lev 27 and 63).
  - `CdRom_FindNextDataSector` adds the range index to the range table as
    integers, index first, through the `CdRangeAddress` pointer/word union
    (`&ranges[i]` and `i + ranges` put the table first in the `addu`).
  - `CdRom_StepReadState` needs no slot macro or pin: the load-delay `nop`
    between `lw D_8009CDA4` and the gp-relative `sw` to `g_CdFadeStartTime`
    is lost only because MASPSX does not know an extern `.sdata` symbol is
    gp-relative. A tentative definition of `g_CdFadeStartTime` with
    `--use-comm-section` gives MASPSX its size, and it keeps the `nop`.
    The same recipe fixes any other `PE1_NOP` before a `$gp` store of a
    just-loaded value.
  - `CdRom_ReadAssetSectors` (pins on `state`, `finished`, `scaledIndex`, `start`,
    barriers on `blocking`/`kind`, the volatile sector-directory reads)
    rebuilds at lev 47 without them. `CdRom_SeekToTrack` keeps the `$s1` pin
    and the count barrier: without the barrier combine knows the count is a
    `>> 22` and turns retail's `blez` into `beqz`, and without the pin
    `slot` and `finished` swap `$s1`/`$s2` (lev 49).
  - `MemCard_PollTransferDelay` keeps the `$a0` pin and the barrier on the
    pending-flag pointer. Without the barrier CSE folds `pending + 1` into
    an absolute address (`lui`/`lw 4`) where retail indexes `$a1` (lev 4);
    `&pending[count - 2]` and `pending -= 2` change the allocation (lev 10
    and 17). The pin is only needed with the barrier.
  - `MemCard_AbortActiveOperation` keeps the barrier that separates
    `name_buf` from `D_8009EE70`: with one value CSE keeps the buffer
    address in `$s0` across `sprintf` instead of rebuilding it (lev 31).
    Passing the buffer to the path format under a second symbol name and
    clearing the retry counter just before the open loop matches with no
    barrier, but that needs the `asm` alias `Save_StartWriteSlot` dropped,
    so the barrier stays.
    `Save_StartWriteSlot` keeps its `$s0` pin and both barriers (lev 23
    without; lev 1 with only the pin).
  - `Entity_AllocBlock` needs no pin or `goto`: a `u8 slot` (the `andi`
    retail applies before each index), an `else if` chain of size classes
    and a `while (slot < 0x10U)` loop. Only the `while` form matches: its
    rotated exit test carries `NOTE_INSN_LOOP_VTOP`, and reorg's
    `mostly_true_jump` predicts a branch to that test as taken, so the
    not-free branch steals the test's `andi` from the target and lands past
    it; a `do`/`while` test has no such note, the branch is predicted not
    taken and its delay slot is filled from the fall-through (lev 1).
    `Scene_UpdateEntityPositions` keeps its `goto link`: duplicating the two
    inlined `Scene_LinkContactTask` calls is not cross-jumped (lev 128).
  - `Entity_TriggerAnimEvent` keeps its three pins (lev 81 without: retail
    copies `entity` to `$a2` first). `Script_RunEntityCommand` needs no pins:
    the plain `g_ScriptNodePool->active = 1; g_SceneDataTable0 -= 0x10;`
    tail allocates `$a1`/`$a0` as retail does.
    `Entity_RelocateSceneData` keeps the `$v0` pin on the loaded block
    (lev 15) and adds region offsets to the database word through the
    `CollisionDatabaseAddress` union: an indexed `regions[i]` walk reorders
    the header loads (lev 9 to 19).
  - `Entity_ResetStateGlobals` clears both ramp edge points by byte offset
    (`g_FloorBlockedEdgePointA`/`D_8009CE0E`); a `FloorEdgePoint` index keeps a separate
    counter and `addiu` base (lev 11). `Entity_ResetAllPools` clears the
    allocation slots by byte offset under `-fno-strength-reduce`.

- Battle (`src/main/battle`, 2026-10-12 pass). The local extern lists, the
  asm symbol aliases (except the one pair below) and the byte-offset field
  macros are gone; the recipes that replaced crutches without changing a
  byte, for the next battle or HUD function:
  - GCC 2.7 CSE drops a cached global after a store through a pointer that
    is not a structure field (`u32 *flags = &core->coreFlags; *flags = ...`)
    and after a QImode store, but not after a structure field store. Retail
    reloads that look like volatile or need a memory barrier usually come
    from one of those two (`Battle_SubActionStep`, `Battle_ActionLifecycle`,
    `Battle_InitEntityColors`, `Battle_ResolveHitOnTimer`).
  - Field-by-field copies of packed words are bitfield assignments
    (`BattleAttackBits`, `BattleTurnBits`, `BattleEffectBits` in
    `battle.h`): a 1-bit field at bit 16 is read through its halfword, which
    is the `lhu` retail has (`Battle_PadStateSyncFlow`,
    `Inv_BuildWeaponList`).
  - Units that read small words absolutely take `-G8` with
    `MASPSX_FORCE_G0` and COMMON definitions of the `$gp` words, plus
    `ASPSX_VERSION: 2.70` when a `$gp` word is a record read at an offset
    (`g_HudOrigin`) or a byte array stored at nonzero offsets
    (`Battle_SetupEntityTarget`, three `PE1_NOP_DEP` barriers retired).
  - `(u16)(s8)byte` at the call reproduces the enemy action-mode argument;
    `(s16)(word >> 16)` and `(u16)(word >> 16)` read the integer half of a
    16.16 field with one `lh`/`lhu`.
  - A base pointer local (`BattleTileSlot *tiles = g_BattleStatusBackgroundTiles`) keeps a
    packet base in a register where retail loads it early.
  - The hit-state, category and reaction inserts into an enemy's core flags
    are `EnemyCoreFlagBits` bitfield assignments; each insert then reuses
    the previous value's register (retail's `$3` chain). The reaction byte
    is read through the `BattleEvasionReaction` record view, which keeps the
    load after the flag stores and the middle store alive
    (`Battle_ResolveHitOnTimer`, one `ApplyEvasionOutcome` helper for all
    four targets; the steal path reaches it by cross-jumping).
  - A narrower copy keeps a register copy alive: `s16 centered = bound`
    stays a `move` where an `s32` copy is folded into its source.
  - Retail fields of the partner render object that splat split into their
    own symbols (`D_800B0D8A`, `D_800B0D88`) are `D_800B0CEC` fields; GCC
    derives the object address from the field address as retail does, and
    only the relocation addend changes. The same holds for the end-of-battle
    panel color bytes (`g_BattleEndPanels[slot].color`).
  - The case-into-if switch shape (`case 0: if (!(rand() & 1)) { case 2:
    ... }`) replaces a goto into a shared resistance body, and a separate
    `if (damage <= 0)` after the scaling block replaces a goto into the
    minimum-damage branch (`Battle_StepEnemyMovement`).
  - An if/else with two calls keeps the branch that `x = 1; if (c) x = 0;
    f(x)` folds to `sltiu` (`Battle_Update` item menu context).
  Crutches left, each removed and rebuilt (lev without it):
  - `Battle_DrawActiveStatus`: the background pointer and the index of the
    last `AddPrim` are both born in `v0`; as pseudos they take `a0`/`v1`
    and the index load moves above the background store (lev 33 and 13).
  - `Battle_DrawEnemyHP` (14 pins, 9 barriers): retail adds the frame and
    digit offsets before the sprite base, which neither typed indexing nor
    a sprite-slot view reproduces (lev 136, byte arithmetic lev 160).
  - `Battle_ProcessActionSlot`: the room argument is set in the delay slot
    of the actor test (lev 1) and the kind is compared against a fresh 1
    where GCC reuses the result register (lev 21). `Battle_UpdateEnemy`:
    a plain copy of the core is folded into `enemy`, swapping the saved
    registers of actor and enemy (lev 129); the shared death step is laid
    out as its own block when duplicated (goto, lev 19).
  - `Battle_StepEscapeOrDeath`: see the `D_800B0D88` views above (lev 42).
  - `Battle_SetupPlayerPalette`: `c2`/`c7` are single-set constants, so
    sched1 launches them late (lev 35).
  - `Battle_AdvanceTurnSlot`: retail shares the queue-index store between
    the loop entry and the loop body; a `while` loop rotates the other way
    (goto and barriers, lev 27).
  - `Battle_DrawStatusPanel` (typed digit packets lev 294) and the four
    phase steps of `Battle_EscapeAndPhaseInitFlow` (62 pins) were not
    reduced further; `Battle_DrawHPBar` is crutch-free (recipes below).
  - Second half of the directory (`Battle_HaltOnPositiveX` to
    `Math_IntSqrt`), measured again after the recipes above:
    - `Battle_StepEnemyMovement` (1 pin): in global allocation the
      resistance mode (7 refs over 137 insns) outranks `enemy` (9 refs
      over 285 insns), so they swap `s4`/`s5` (lev 22). Naming `enemy`
      instead of its copy in one more read makes it win (lev 1, but that
      load then uses `s4` where retail uses the copy in `s2`); moving the
      mode's initialisation or retyping it does not help (lev 18 to 71).
    - Constant hoisting (`Battle_Update` gauge recolor: 6 pins and 7
      barriers; `Battle_StepPostBattle` cases 2 and 4: 6 pins, 3 barriers
      and the volatile panel view, measured one at a time lev 2 to 38;
      the `Battle_StepVictory` stat reset and `Battle_StepPostBattle` case
      3 are solved, see the recipes after this list): retail loads
      every color or edge constant at the top of its block and leaves the
      later load-delay slots empty. Without the pins sched1 places each
      `li` in the first load stall after its use, whether the constant is
      a block local, a function-scope variable set in two cases (no longer
      LAUNCH priority, still a stall filler) or a `u8` (lev 75, 60 and 103
      for the three blocks). The `asm("" : : "i"(n))` barriers in
      `Battle_Update` also keep cross-jumping to the last store only;
      without them blocks 1 and 3 merge (lev 299).
      `Battle_StepPostBattle` case 4 written plainly (`panels` local, the
      quad corners through `BATTLE_SET_QUAD_XY`, no volatile view, no
      pins) is lev 8: retail allocates the fade byte before the 220 corner,
      which it can only do if the `li 220` sits at the top of the block
      (not launched); sharing `rightEdge` or `horizontalEdge` with case 2
      to make it multi-set moves the other registers (lev 31 to 47).
    - `Battle_StepAyaAction` (6 pins, 3 barriers left; cases 0 and 9 are
      plain C, see the recipes after this list): case 11 restores the pose
      through the position and rotation records at lev 41, all of it one
      `lh` of `posZ` that sched1 hoists into an earlier load stall where
      retail reads it after the rotation stores (a `z` local, a reload
      through `D_8009D254` or a rotation record copy do not keep it there).
      Case 6 without the product barrier is lev 8 (`mflo` lands in `t1`, not
      `v1`; inline products, two locals, `<< 4` and operand order do not
      change it; the second product in the function-scope `slot` is lev 7,
      the permuter found nothing closer in 5800 tries). Case 15 written as `flags &= mask` with `mask` assigned
      twice keeps the two `and`s (lev 11; the one-statement mask folds them,
      lev 29), but the flags load moves above the visibility store and the
      registers rotate.
    - Stack reserves: `Battle_Update` keeps 0x1A0 and
      `Battle_StepPostBattle` 24 unused frame bytes; no local of the
      function accounts for them.
  - Recipes found in the 2026-10-12 battle debt pass (each replaced pins,
    barriers, gotos, casts or volatile with the same bytes):
    - A byte computed in steps belongs in a wider variable truncated by the
      expression itself: `u16 brightness; brightness = ~(brightness << 3) &
      0xFF;` keeps the load, shift, `nor` and `andi` in one register, so
      cross-jumping shares the `andi` and the fade calls between cases 1
      and 5 (`Battle_StepVictory`; a `u8` variable gives the temporaries
      their own register, lev 9).
    - A callee-saved pointer that two cases set separately is block-local in
      each case (`partner` in `Battle_StepVictory`): as one function-scope
      variable it is multi-set and global allocation swaps it with the
      constant 1 kept across the calls (lev 13).
    - A result flag that retail sets in the delay slot of the last branch is
      assigned after the conditional call (`if (...) f(); result = 1;`),
      not before it (lev 3).
    - A constant that retail loads at the very top of its block, far from
      its use, is a variable set to the constant and then updated in place
      (`brightness = 120; brightness -= g_BattleEndTimer * 4;`): set twice,
      the pseudo is not launched by sched1, so its `li` stays where the
      source puts it (`Battle_StepPostBattle` case 2, which retired the
      `brightnessBase` pin; the `top` constant beside it is stored as is and
      still needs its pin, lev 23 without).
    - Loops that walk the entity list in different cases of one function
      share the cursor variable when retail keeps every walk in the same
      callee-saved register: a walk without calls then still takes `s0`
      (`Battle_StepPostBattle` case 1, which retired the `hiddenEntity`
      pin).
    - Packet arrays are typed (`RenderPagedSpritePacket g_BattleHpDigits[2][4]`
      and the rest of `battle_status.h`) and indexed naturally. When retail
      keeps the sprite base (array + 8) in its own register and adds the
      offset to it, the stores go through `BATTLE_SPRITE_SLOTS`, a cast of
      `&array[0][0].sprite` to rows of `BattleDigitSlot`: as a memory
      reference GCC adds the shared index offset to the hoisted `array + 8`
      while the packet argument and the clut store keep `array` itself
      (`Battle_DrawHPBar`, six `goto` loops, five pins, two barriers and
      eight pointer casts retired). An assignment as the stored value
      (`tile.r0 = shade = shades[i]`) expands the store address before the
      load, which is retail's order.
    - A statement macro instead of a `static inline` sound helper:
      inlining gives each `g_AkaoBgmHandle[0]` read its own `lui`, the
      direct code takes the table address into a register once, as retail
      does (`PLAY_TABLE_SOUND` in `Battle_TargetAndEnemyTurnFlow`, which
      retired the four volatile handle pointers, two pins and two barriers).
    - Globals that retail reaches as one object are records:
      `g_AyaSavedPos` (three words) and `g_AyaSavedRotation` (three
      halfwords) keep the player loads and the saved-pose stores
      interleaved, because GCC 2.7 lets a structure store alias any other
      structure access but never a scalar global. The evasion outcome is
      `g_EvasionOutcome` (`BattleEvasionOutcome`, at `g_EvasionStatus`):
      its reaction is read back after the core flag bitfield stores. Under
      cc1 `-G0` the record address is CSE'd into a register (lev 57), so
      the unit moved to cc1 `-G8` with `MASPSX_FORCE_G0`, which leaves the
      other words absolute (`Battle_StepAyaAction`). The two byte names
      remain for `Battle_ResolveHitOnTimer`: its ASPSX 2.56 addresses
      `g_EvasionOutcome+1` absolutely, ASPSX 2.70 or later through `$gp`.
    - Constant gradients and colors are `BATTLE_SET_GAUGE_GRADIENT` and
      `BATTLE_SET_SPRITE_COLOR` on the typed arrays: GCC sorts the byte
      stores by value itself (`Battle_StepVictory`, 6 pins and the interior
      byte symbols). Quad corners are `BATTLE_SET_QUAD_XY` on a pointer
      formed from a local copy of the array base (`panels = g_BattleEndPanels;
      panel = &panels[slot]`), which loads the base where retail does
      (`Battle_StepPostBattle` case 3, the volatile view, two barriers and
      five pins).
    - An `s16` parameter on an inline helper keeps CSE from reusing a
      caller's `index - 1` for the helper's own `initial - 1`: the
      conversion gives the helper a separate value, and it folds away
      afterwards (`RewindTurn` in `Battle_AdvanceTurnSlot`, which retired
      the `savedIndex` barrier; `int` and `u8` parameters keep lev 26).

### Small-data recipes that replaced crutches (2026-10-12 pass)

These apply to any unit, not only the ones they were found in:

- Loop exit tests. When a loop body starts with a conditional exit,
  `expand_end_loop` rolls that test to the bottom and jumps to it, and the
  first jump pass then copies the test in front of the loop, so the first
  iteration is peeled. Retail search loops that test at the top (written
  with a backward `goto` until now) come out of a plain loop whose body
  opens with a block-scope declaration: the block note stops the roll
  (`Scene_LoadMap`: `do { u8 id = walk->...; if (id == sceneType) break;
  ... } while (i < count)`). A byte compared inside such a loop must be
  held in a full-word variable, or loop.c hoists its zero-extension to the
  loop entry and the load moves with it. The other way round, a loop that
  retail enters through a jump to its bottom test (`Menu_StepItemSelectScreen`)
  cannot come from `while`/`for`: the jump pass copies the test in front of
  the loop and cse folds it against the constant start value (lev 28);
  an inline helper in the condition stops both the roll and the copy
  (lev 8) and a `do`/`while` has no entry jump (lev 7).
- A `PE1_NOP`/`PE1_NOP_IO2_DEP` slot between a load and a `$gp` store of
  the loaded value stands in for the load-delay `nop` ASPSX emitted. A
  tentative definition of the stored word in the unit plus
  `--use-comm-section` gives MASPSX its size and it keeps the `nop`
  (`CdRom_StepReadState`, `Scene_LoadMap`, `Battle_StepCharacterAction`).
- 16-byte `struct { char _[16]; } X __asm__("sym")` views in a `-G8` unit
  only make cc1 treat a small word as absolute. `MASPSX_FORCE_G0: 1` with
  `--use-comm-section` and tentative definitions of the words retail reaches
  through `$gp` makes every other word absolute, so plain declarations give
  the same code (`Battle_BuildTargetList`, `Battle_Init`,
  `battle/player_turn.c`, `Battle_FinalizeAttackResult`,
  `Battle_AdvancePhase`, the forced-G0 task units).
- Views that only separate reads of one word for CSE give way to one-field
  record views (`pe1/battle_word_records.h`, `pe1/draw_slot_record.h`,
  `g_AyaBonusPoolRecord`, compare `room_sound_slot.h`). A record read is a
  structure access: GCC reloads it after a store through a structure
  pointer, where a scalar at a constant address is assumed not to alias and
  is reused. At one word the record is small data, so a -G8 unit addresses
  it directly; under `-G0` the unit switches to cc1 `-G8` with
  `MASPSX_FORCE_G0` so the record does not get its address CSE'd into a
  register (`Battle_DrawSpellName`). The linker map names the record at the
  word's address. `Battle_ApplySpellEffect` lost 33 views this way.
- The reverse case, a structure field that retail reads as a plain scalar,
  needs no byte offset either. GCC 2.7 marks a MEM as "in struct" for a
  member access, for an indexed pointer (`p[1]`, an INDIRECT_REF of a
  PLUS_EXPR) and for `*&aggregate`; such a load may be moved across a store
  to a fixed global. A load through a pointer variable is a scalar MEM and
  stays ordered against those stores. `Pe1Fixed_IntegerHalf`
  (`pe1/fixed_point.h`) reads the integer half of a 16.16 coordinate that
  way; `Menu_MainUpdate` lost its three `M2C_FIELD` position reads with it.
  The same split explains why a store and an immediate reload of one field
  survive CSE only when the two addresses are different RTL
  (`Battle_ApplyEnemyAttack`: reloading `stateFlags` through the
  function-entry `&g_ActiveActor->stateFlags` pointer keeps the reload, but
  in s0 instead of retail's a0, lev 1; a pointer to the field taken in the
  same block is CSE'd away, lev 52).
- Scalar views do the opposite job: `g_EquippedWeaponSlot`/`ArmorSlot`
  (`pe1/aya.h`) are byte scalars, so in a forced-G0 -G8 unit every slot
  access is a direct `lui`/`lb` or `lui at`/`sb` instead of an address held
  in a register (`Inv_TransferItem`, `Inv_RemoveActiveListItem`).
- A masked read-modify-write chain with hoisted `lui`/`ori` masks, or a word
  stored and immediately loaded again, is a bit-field store in the original:
  `g_BattleResultFlags` (Scan, `BattleResultFlagsRecord.bits`),
  `Combatant.stateFlags` (`COMBATANT_STATE_BITS`, Haste) and the
  `BattleTurnBits.actions` read (a bit-field read loads the word and masks
  it, where `turnWord & 0xF` is narrowed to `lbu`) replace volatile casts.
- A read after an outgoing stack argument store is reloaded only when it is
  a structure access: `g_AkaoBgmHandle[0]` (array element) replaces the
  `void * volatile *` pointers of the menu sound helpers and
  `Menu_MainUpdate`.
- An inline helper keeps an index scaled before the table base is added:
  `&D_800A1E64[kind - 5]` written directly folds the -5 into the address
  (lev 16); `AmmoPoolRecord(kind - 5)` does not (`item/armor_capacity.c`).
- Under `-G0` every extern is absolute anyway; the views in
  `Scene_UpdateBgDraw` and `Scene_LoadSceneData` were unnecessary.
- An array bound changes the small-data decision: `g_ScriptNodeSeqCounter[1]`
  is small, `[]` is not. Declaring a symbol as an incomplete array before
  its tentative definition in the same unit also makes it large, so
  `boot/frame_loop.c` includes `pe1/boot.h` after its definitions.
- `CdRom_PollReady` is an `extern inline` in `pe1/cd_poll.h`: the boot disc
  loaders inline it as retail does (the copy into the return register is
  what the old `$v0` pins and status barriers imitated), and
  `cdrom/scene_assets.c` keeps the out-of-line definition.
- A loop that walks a table by index rather than an incremented pointer
  leaves the table pointer single-set, and sched1 then places its address
  load late (`MemCard_InitManager`, no pins).
- Pointer/word unions (`CdRangeAddress`, `PeSectorTableAddress`,
  `SceneCopyCursor`) replace `(u32)` casts where retail adds an index to a
  table address with the index as the first `addu` operand.
- Overlays link against their own symbol maps, so a header symbol whose
  `__asm__` label is the name overlays use (`g_ActiveDrawSlot` in
  `boot_display.h`) keeps the label until the overlays are renamed too.

### Crutches left in scene, task, gpu, item, boot and main (2026-10-12 pass)

Each site was removed on its own and rebuilt; the lev given is for that
single removal (none was byte-identical):

- scene: `Scene_LoadSceneData` keeps the `$t0` `copyEnd` pin (lev 5) and the
  destination-word barrier (lev 2). `Scene_LoadEntityTexture` keeps the
  `$s0` loop pin (lev 9), the record barrier (lev 3), the packed-word
  barrier (lev 9) and its stack reserve. `Scene_TickTimers` keeps the `$a1`
  entry pin (lev 30). `Scene_UpdatePlayerEntity`/`Scene_UpdateEntityFacing`/
  `Scene_UpdateEntityFacingFromPad` keep their angle launders (lev 1 each:
  CSE folds the masked angle into the previous one) and the pad memory
  barrier (lev 7). Retry `goto`s in the room loaders stay for the loop.c
  reason given for the CD loops.
- task and gpu: see "Task and GPU units (src/main/{task,gpu})" below.
- item: `Inv_SetActiveList` (lev 9 to 36), `Inv_GetSlotItemData`/
  `Inv_GetActiveSlotCount`/`Inv_IsSlotEquipped`/`Inv_IsSlotSelectable` empty
  asm statements (lev 18 to 61: they stop the shared slot lookup being
  cross-jumped), `Inv_ClearDisplaySlots` (lev 12),
  `Inv_CompareItemsForSort` (two memory barriers, lev 22 and 4; moving the
  function-address load among the three reads does not help, lev 22),
  `Inv_CheckItemEquippable` (lev 5 to 40), `Inv_FindFirstEmptySlot`
  (lev 16, 49), `Inv_RebuildSelectionBitset` (lev 17 to 36),
  `Aya_StatQueryLevelAndSubLevel` (lev 65). `Inv_EquipItemById` (formerly
  `Inv_FindItem`) and `Inv_InitMaxLevelInventory` are crutch-free.
  `Inv_TransferItem` lost its three `goto`s: every arm of the `else if`
  chain calls `Inv_RebuildSelectableMask` itself and the two refusal arms
  set `result = 0` in an `else`, with one `return result` at the end;
  cross-jumping folds the copies into the last arm, which is retail's
  layout (rebuild, then the refusal store falling into the return).
  `Inv_SetActiveList` case 5 keeps its pins: the
  natural form allocates the action pointer to a3 and the selection to a1
  (lev 37). `Inv_RemoveActiveListItem` keeps one m2c integer that holds
  first the list pointer and then the removed item (typed forms swap
  v0/v1, lev 2). The six local item
  record lookups became `Inv_LookupItemRecord`/`Inv_LookupItemRecordScaled`
  in `pe1/inventory_slots.h`.
- boot: `Field_HandleStateTransition` (lev 13 to 48), `Draw_InitPipeline`
  (pins lev 11 to 13, barrier lev 37: the constant clamps are not folded
  without it). `Boot_DiscTableLoad` keeps its retry `goto`s (loop.c).
- main: `Battle_ApplySpellEffect` is crutch-free. `Battle_BuildTargetList`
  `finalCount` pin and barrier (lev 5), `Battle_DrawStatusOverlay` (lev 2
  to 7), `Battle_StepCharacterAction` (lev 2 to 47; the queue offset pin
  alone lev 3, a structured rewrite with record views lev 216 because the
  retail block order follows its `goto`s),
  `Battle_UpdatePlayerTurn`/`Battle_ApplyPlayerHit` (lev 11 to 20),
  `Battle_DrawTargetHighlight` (lev 2 to 24), `Battle_DrawSpellName` (lev 2
  to 15; a typed `BattleGaugePrim [2][10]` rewrite lev 224: retail keeps the
  packet base in a register). `Battle_AdvancePhase` keeps the copy of the
  sound table as m2c wrote GCC's block move (the struct assignment matches,
  but the target search's found block then moves out of the copy, where
  loop.c placed it in retail), its `goto`s, and the scaled queue reads in
  the two finish tails (array indexing gives the index another register,
  lev 6); those reads and the block-move alignment test now go through
  block-scope `BattleInitSlotAddress`/`CopyAddress` unions instead of byte
  and integer casts. `Battle_DrawStatusValue` and
  `Battle_DrawDecimalNumber` add their precomputed byte offsets to a
  block-scope `BattleStatusPrimAddress` (`pe1/battle_status.h`) loaded with
  the base right where the sum is needed. A function-wide union copy of the
  `buffer` argument is allocated before the other saved values (s4/s5
  swap, lev 13); the short-lived copy stays out of global allocation, and
  a copy of a constant base folds back into `%lo(symbol)(index)`
  addressing. The x read keeps the `D_8009E360` name: one base symbol for
  both halfwords makes CSE share the address (lev 8).
  `Battle_DrawStatusSymbol` and `Battle_DrawATBGauge` address their ordering
  entry and sprite as array elements. `Battle_ApplyEnemyAttack` keeps
  two volatile `stateFlags` reloads: bit-field forms drop the store and
  reload pair (lev 52 with branch offsets, 2 instructions). `Battle_UpdateEntityFacing` has no `goto`s:
  the facing mode is passed as a constant in nested `if`s with one
  `Entity_SetActionMode` call per arm, which cross-jumping merges into the
  retail `li a1,1` / `move a1,zero` pair; a `mode` variable becomes an
  `slti` store-flag and `&&` folds into an unsigned range test (lev 16
  to 21). `Battle_DrawStatusOverlay`
  case 3 keeps its packet base pin (typed sprite forms lev 7 to 23).

### Battle HUD recipes and crutches left (src/main/battle, first half, 2026-10-12)

Recipes that removed crutches here and apply to other HUD code:

- PSY-Q macro shape. Retail indexes the draw-buffer packet arrays through
  `(&array[D_8009CDDC][i])->field`, the shape the `setXY0`/`setRGB0` macros
  expand to. GCC turns that into pointer arithmetic, `(f*stride + i*size) +
  base`, and keeps the base in a register across a loop. A direct
  `array[f][i].field` gives `(i*size + f*stride)` with the symbol folded into
  the store (`MAX_HP_DIGIT`, `PANEL_DIGIT`). When retail adds the base before
  the store offset of a sprite, declare the sprite-first view
  (`BattleDigitSlot`) so the symbol is the sprite, not the packet.
- Branches that store the same tail (`.b = 128` in both colour branches,
  the four background tiles) should repeat it in each branch: retail's
  shared block is GCC cross-jumping, not source.
- HUD colour resets that use constant indices (`[0]`, `[1]`) are plain
  `BATTLE_SET_GAUGE_GRADIENT`/`BATTLE_SET_SPRITE_COLOR` calls in the natural
  per-quad order. The scheduler reorders the stores and groups the constants
  the way retail does. The macros are comma expressions: a `do { } while (0)`
  body or an inline function changes the result (lev 96 and 62).
- A test of the form `if (D == 0 || ...) return; do { D--; } while (...)`
  is the inverted loop that retail's entry jump comes from (`RewindTurn`).
- A copy register that retail keeps for one pointer (`statusCore`) comes
  from reading the field twice, before any store.
- A temporary that only feeds two stores and must not share a register with
  an earlier variable is block-local (`finisher` in
  `Battle_PhaseInitEnemyTurn`).
- A result flag that must not be merged with the constant 1 by CSE is an
  `s8` (`Battle_ProcessActionSlot`).

Crutches left; each was removed on its own and rebuilt, lev for that removal
or for the best crutch-free form tried:

- `battle/encounter.c`: the per-frame gauge colour cases in
  `Battle_PhaseInitEnemyTurn`, `Battle_PhaseHitReaction` and
  `Battle_PhaseEndTurn` (all register pins and empty barriers of those
  functions). Retail loads all five gradient constants before the third
  store of each case; every crutch-free form tried (struct fields, the
  gradient macros, shared or block-local variables, `u8` constants, an
  inline helper, a volatile draw slot, even `-fno-schedule-insns`) loads each
  constant right before its first use (lev 20 to 60 per case, 260 for the
  four HP cases together). The `asm("" : : "i"(n))` statements stop
  cross-jumping from merging more than the last store of each case.
  Measured again on a one-function copy of a case (2026-10-12): plain code
  already gets retail's registers (the last-born constant in `a0`), and a
  loop note after the first store (`do { ... } while (0)`, which sched1
  treats as a full barrier) gets retail's schedule, but the loop weights
  the references inside it and the constants then take `a0` upwards in
  birth order (lev 18 for the best of 288 barrier placements, types,
  declaration orders, `register` and inline-parameter forms). No single C
  form gave both.
  Measured on the whole of `Battle_PhaseHitReaction` (2026-10-12, third
  pass): the plain macro form is lev 241 for its four ATB cases. In retail
  the constants that are not stored first sit in the load stalls of
  exactly the first two stores (ATB) or the first store (PE gauge), the
  rest at the top, and no later stall is filled; sched1 only does that when
  no constant is ready before a full barrier after those stores, so the
  source had something that sched1 treats as a barrier there (a loop note
  or an asm). The register order (last-born constant in `a0`, the first
  in `t0`) needs the later constants to have the higher allocation
  priority, which the local-alloc formula only gives them when all five
  die together; a loop note after the second store reverses it (lev 93
  ending the loop there, 169 starting it there, 80 with both), shared
  function-scope colour variables move every register (lev 189 to 247), a
  copy into block-local `u8`s lev 109, a `static inline` helper with the
  whole if-chain is the plain lev 241. A non-volatile unused array gives
  the same frame as the `volatile` reserve, so the reserves are unused
  locals of the original, but no role for them is known.
  `moveLock` in `Battle_PhaseInitEnemyTurn` (lev 11, every statement order
  and a struct view of the flag word alike: the flag store is a fixed
  scalar and the motion stores are struct fields at a varying address, so
  sched1 sees no dependence and sinks it below them). The memory barriers in
  `Battle_StepEscapeOrDeath` and `Battle_StepEntityDeath` (lev 41: retail
  reloads `entityFlags` after storing it). The three stack reserves are
  retail's unused frames.
- `Battle_DrawStatusPanel`: the MISS branch keeps seven pins and two
  barriers. Its five byte stores need the folded symbol and `f*0x578 +
  s*0x8C` order; the macro form wastes 8 to 48 stack bytes because combine
  leaves `(use)` insns for the deleted base registers, the direct array form
  computes the slot offset first (lev 73 to 128).
- `battle/turn.c`: `room` (`$4`, lev 27: one delay slot,
  the rest is branch offsets), the `actor` barrier (lev 129: global
  allocation swaps s0/s1), the `stepDeathAnimation` goto (lev 184 for an
  inline search helper).
- `Battle_DrawStatusPanel`: the MISS branch with the panel as
  `(g_BattlePanelDigits[frame] + slot)` computes the frame offset first as
  retail does, but the five byte stores then leave five `(use)` insns
  after combine and 40 unused frame bytes (lev 77; the `(use)` comes from
  combine's REG_DEAD note placement when the base pseudo of a three-insn
  combination disappears).

### Task and GPU units (src/main/{task,gpu})

Recipes that removed crutches here (2026-10-12, second pass). Each one was
found by rewriting the whole function from the retail listing rather than
deleting one pin at a time:

- A dispatch `switch` whose decision tree splits at the wrong case is
  missing a case. `Script_DispatchScriptCmd` has a no-op opcode 2100
  (`case 2100: break;`): with it the tree pivots at 2500 as retail does and
  the 23-label `goto` tree became a plain switch. Count the case nodes
  `balance_case_nodes` sees before blaming the compiler.
- Two branches that end in the same store sequence are cross-jumped into
  one tail when each branch does the whole update itself
  (`Script_SetObjEntryFlag80`/`Script_ClearObjEntryFlag80`: each table branch
  computes its triangle as `index * size + table` through the
  `CollisionTriangleTable` word and sets the flag; the shared `addu`/`lbu`
  tail is the compiler's, not the source's).
- A copy of a value into a second variable, or reuse of one variable for
  two roles, shows up as retail's register reuse:
  `Script_TurnTowardPointStep` turns its target coordinates into the deltas
  passed to `ratan2` (`x = (pos_x - x) >> 16`), and its wrap tests use a
  fresh block-local `wrap`.
- Store order of several fields follows the natural statement order once
  the value has one name (`Script_SnapPosToSpawnPos` stores x, y, z).
- Block-local single-set temporaries (an inline `AdvanceCursor(dx)` that
  reads x and y and writes both back) replace function-scope `x`/`y`
  reused across calls; `Draw_PrintTimeValue` lost both memory barriers.
- Inline helpers with their own parameter copies reproduce retail's
  copies: `Draw_PrintCenteredTextInWidth` is `PushCursor`, the inline
  `Draw_MeasureTextWidth`, a `SetCursor`, a print loop that saves the
  known cursor (`SaveCursor`) and two `PopCursor`s.
- PSY-Q GTE macros cover the hand-pinned GTE sequences:
  `gte_SetBackColor`, `gte_SetColorMatrix` (`Boot_SetBackColor`,
  `Boot_SetLightColor`) and `gte_MulMatrix0` (`Script_SetGteMatrix`).
- Address arithmetic on a pointer as an integer goes through a union
  (`RelocAddress`, `ScriptNodeAddress`, `DrawPrimLink`, `TimRelativeTable`).
  Declare the union in the innermost block that uses it: a union declared
  at function scope gets the older pseudo, CSE then rewrites the packet
  pointer's later uses through the copy and an extra `move` appears.
- Plain argument use beats a laundered copy: `Script_RunInvCommand` and
  `Script_OpenMemCardDialog` match with the argument block and the shared
  `return 1` written directly.
- `sll`/`sra 16` before every use of a value, plus plain `move`s between
  copies of it, is a `short` local: GCC 2.7.2 on MIPS does not promote
  HImode variables, so each use as `int` re-extends it, while assigning a
  masked `int` to it is a bare copy. `Render_CameraStepAngleFade` lost 8 pins,
  4 barriers and 7 `goto`s this way (`short current, target, step`, with an
  `int` copy of the angle and target for the first two reach tests and the
  wrap test written as `(sum & 0xFFF) <= target`).
- GCC 2.7.2's alias rule says a scalar at a fixed address never conflicts
  with a structure member reached through a pointer. A load of such a scalar
  is hoisted out of loops and scheduled across member stores; a record view
  of the scalar (`g_PmTables`, a one-element array like
  `g_ScriptNodeSeqCounter[1]`) keeps it in place without volatile.
- A stored packet field read straight back is reloaded by itself: any store
  through a pointer invalidates every remembered memory value in CSE, so
  `Draw_EmitGlyph` and `Draw_EmitDigitSprite` need no volatile views for
  their vertex reloads. A reload of a value stored in both arms of an `if`
  comes from cross-jumping the two stores into the join
  (`Draw_AllocColorTri`'s pulse shade).
- PSY-Q `addPrim` is the 24-bit tag bit-field (`RenderPacketTag.link`):
  `p->tag.link.address = ot->link.address; link.packet = p;
  ot->link.address = link.word;` reproduces the mask-and-merge sequence, and
  for `Draw_AllocSprite`, `Draw_EmitGlyph`, `Draw_AllocTexturedQuad` and the
  wipe triangles it replaced the tag pins and barriers. The `setRGB0`
  colour store with the code byte kept is `RenderPacketColor.fields.rgb`.
- A `case` that ends with the same `return` as the next label falls into
  it: `Boot_MemCardStepInit` state 0x3B shares `default: return 1` instead
  of an empty barrier.
- Allocations written in their own blocks (`u8 *old`, `u8 *next` declared
  inside) give retail's registers where function-scope temporaries reused
  for the second allocation do not (`Draw_AllocSprite`).
- A variable reused for a second role is retail's register reuse:
  `Render_CameraTrackEntityAngle` keeps the remaining distance in
  `angle_difference`.

Crutches still in these units, with the best crutch-free lev found:

- `Draw_InitGpuHwRegs` (7 pins, 2 barriers) and `Draw_FlushPrimQueue`
  (10 pins, 4 barriers) are the additive generator around the
  80-byte `.text` state block at 0x80070E04, the same original-assembler
  module as `Draw_PackPrimColor` (see ASM_AND_GTE_POLICY.md): absolute
  addresses built with `lui`/`ori`, constants loaded with `ori`, `or` moves,
  no frame, hand-filled delay slots. GCC emits none of these (crutch-free
  lev 22 and 24). They should become `GAME_ASM_FUNCTION` units with
  evidence entries in a reviewed change; this pass adds no assembler. The
  queue offsets are now added through the `DrawPrimGeneratorAddress` union
  instead of integer casts.
- `Boot_MemCardStepInit`: 3 re-dispatch `goto`s (a `for`/`continue` loop
  lets loop.c hoist `0x400000` and `1`, lev 39; tail recursion re-runs the
  `&g_GameState` load after the re-entry label, lev 24). Its empty barrier
  is gone: state 0x3B falls into the `default: return 1`, which shares the
  `li v0,1` jump as retail does.
- `Script_LoadSceneData`: 2 CD retry `goto`s (a loop lets loop.c hoist the
  `-1` comparands into saved registers, lev 32).
- `Akao_ProcessCommand`: 1 `goto archive_match`; retail places the match
  block after case 0x199, a structured `break`/flag places it in the loop.
- `Script_SpawnChildNode` (6 pins, 2 barriers): crutch-free lev 19. Viewing
  the free-list head as a one-element array (the `g_ScriptNodeSeqCounter[1]`
  recipe) fixes the store order the memory barrier holds and the node
  register (lev 12 with no else-branch crutch, 8 with the `$4` state pin);
  what remains is the `g_CurrentEntity` load, which retail schedules right
  after the argument load in both branches.
- `Script_SetEntityActionAndWait` (1 pin): lev 2 to 3; retail loads the
  current frame first into `v0`, every crutch-free form either loads it
  second or allocates it to `v1`.
- `Script_WaitAnimRange` (1 pin): instruction sequence exact, lev 19 from the
  global allocation of `next`/`target`/`count` (retail `a0`/`a2`/`v0`).
- `Render_CameraTrackEntityAngle` keeps its 8-byte `stack_pad[2]` frame reserve
  (lev 18 without it).
- `Draw_AllocColorQuad` (13 pins, 5 barriers): with every crutch removed
  the instruction sequence is identical and only the register assignment
  differs (lev 138: the first quad in `s1` with `height` in `s0`, the
  packet copy in `t2`, the masks in `a0`/`a2`); declaration order, block
  scope for the first quad and dropping the `w`/`h` copies do not change
  the global allocation order (lev 138 to 153).
- `draw/text.c`: `Draw_EmitGlyph` keeps the `$4` vertex pointer
  pin and the descriptor barrier (lev 44 without them: retail copies the
  packet into `a0` for the vertex stores, an inline vertex helper or a
  block-local copy is merged back into the packet register);
  `Draw_AllocTexturedQuad` keeps the `$4` GetTPage argument pin (lev 2:
  `move a0,zero` is scheduled later), the `$5` half and `$6` x pins and
  three barriers (lev 5 to 15 each; `spacing >>= 1` in place of `half` lev
  16); the `DecodeGlyph` `$4` pin stays (a `u8` copy of the code lev 10, a
  `u8` parameter lev 14: retail masks the code byte again for every use but
  keeps the masked glyph in `v1`).
- `draw/wipe.c`: `Draw_AllocColorTri` keeps the tag tail of its
  tile, one pin and two barriers (lev 27 without them: the code byte load
  is scheduled above the width store and the cursor loads move); its pulse
  shade and colour clear are plain C now.
  Plain field statements (`w`, `h`, `code |= 2`, `y0`, `x0`, then the
  AddPrim pair with `old = D_8009D100` after it) reach lev 16: the tag load
  and the semi-transparency byte load still move below the width store
  (same base register, disjoint offsets, so sched1 may reorder them), and
  the x/y cursor stores swap.
- `Draw_InitDisplay` (3 pins, 1 barrier) repeats `Boot_InitMemoryLayout`
  (sys, also pinned) on the `u8 *volatile` buffer words of `pe1/boot_services.h`;
  both should change together with that header. Its locals are `u8 *` now
  (no integer casts). Without the barrier lev 14 (the `g_OtBufferTable`
  address joins the `g_OtBufferTable[0]` store and loads late, and the
  constant 1 is rematerialized before the loop); without pins and barrier
  lev 61; calling the layout as a `static inline` function lets cse fold
  the pointer chain into symbol offsets (lev 63).
- `battle/status_hud.c`: the status-icon packets (`D_8009E960[2][13]`,
  `D_8009EC38[2]`, 28 bytes: draw mode then sprite) are addressed from the
  sprite part at +8 (`D_8009E968`, `D_8009EC40`) with byte offsets (7 byte
  additions, 2 raw offsets) and `Draw_StatusIcons` keeps 8 pins. A typed
  `{ tag; mode; RenderSpritePacket sprite; }` array reproduces every
  instruction except the register base, which retail points at the sprite
  part (lev 8 from the immediates). `battle_status.h` now types
  `D_8009E960` as `RenderPagedSpritePacket[2][13]` for `Battle_DrawHPBar`;
  `D_8009E976` and `D_8009EC38` are still byte arrays for this unit.
- `battle/status_hud.c`: the status icons are typed now
  (`g_StatusIconPackets[2][13]`, `g_StatusMarkerPackets[2]` in
  `pe1/draw_state.h`, linker names beside the battle byte arrays), and
  `Draw_SetupSprites`/`Draw_QueuePrimitive` are plain field accesses.
  `Draw_StatusIcons` keeps 7 pins and the integer address sums in its
  three emit macros (3 byte additions): retail adds the slot and icon
  offsets to the sprite base and derives the packet base as sprite base
  minus 8, which typed indexing does not produce (lev 48 with a texture-page
  step back from a `RenderSpritePageSlot` view, 76 with paired array
  references, 188 to 237 for a single typed macro with `short` or `int`
  cursors). Typed `&g_StatusIconPackets[slot][code].sprite` plus a separate
  packet address is merged by cse into one register and stores at +16
  (lev 106 to 110 for the first emit): retail folds the +8 of the sprite
  part into the symbol constant and derives the packet as that constant
  minus 8.

### Casts that hide type facts (2026-10-13)

The two unnecessary-cast rounds left casts whose removal changes the
object or the diagnostics. Those that hid a wrong declaration were fixed
from the retail code: the chained AKAO opcode handlers take the
`(AkaoTrack *, u32 voice_mask)` table signature, `RoomEnt.sub.cb` takes the
entity, `Render_GetOrLoadFontGlyph` returns `int`, `Combatant.statusFlags2`
is `u32`, `gte_ldv0_word3` takes the `s32` translation, the timer tick
walks `g_GameTimers` instead of an interior volatile word, narrowing reads
of wider members are value conversions (`(u16)`, `(short)`; GCC still
emits `lhu`/`lh`) instead of pointer views, and
`Akao_SendPositionalCmdStereo`, `MenuWidget_InitPool`,
`MenuWidget_NavScrollTo`, `Save_SetTitleStyleFlag` and one
`Entity_SetActionMode` call use their prototypes. What is left, with the
evidence:

- **`Menu_StepItemSelectScreen` case 3 (`menu/status_panels.c`)** calls the
  `void` `Menu_StepSkillList` through an `int`-returning view. The callee
  never writes `$v0` (its epilogue is `lw ra` ... `jr ra` after
  `Menu_PlayConfirmSound`), so the true type is `void`. Retail keeps cases
  2 and 3 as two identical tails (`0x80043ECC` and `0x80043F18`:
  `Menu_CreateInventoryListView`, `Menu_CreateEquipItemSelectionView`,
  `MenuWidget_FindByModeAndSelectedBase(2, 5)`, `Menu_StepSkillList(..., 0)`)
  that differ only in the `g_InvItemUsableFlag` store before them; with a
  `void` call GCC's post-reload cross-jumping merges the tails. The view
  makes the second call `(set v0 (call ...))` so the patterns differ. No
  plain C shape that keeps the two tails apart has been found.
- **`Menu_MainUpdate` (`menu/main_update.c`), `Entity_SetActionMode(...,
  0x12)`** passes `var_a2` as a third argument. `Entity_SetActionMode` never
  reads `$a2`; retail's `$a2` is the last slot-colour byte the function
  stored. Dropping the argument shortens `var_a2`'s life and local-alloc
  swaps `var_a1`/`var_a2` over the whole colour block (lev 112). The block
  writes the four `rgb` triples of a 0x48-byte slot record at
  `D_800B0158 + g_ActiveDrawSlot * 0x48` through `register` locals; it has
  to be rewritten as typed colour stores before the call can be plain.
- **`Combatant.curHP` (`pe1/battle.h`, `u16`)**: `Battle_ApplyEnemyAttack`
  reads it with `lh` through three `s16` views and once as
  `lhu`/`sll 16`/`sra 16`, and stores `-1` into it. Declared `s16`, every
  unit compiles the same if that one read is written `(u16)pD->curHP << 16`;
  but the `EnemyCombatant` view of the same word has to stay `u16`
  (`battle/turn.c` and `battle/context.c` read it with `lhu`). The field
  keeps `u16` until both views can agree.
- **`Akao_StepSequencerVoice` (`akao/voice_bank.c`)** writes the bank masks
  (`key_off_request_mask`, `active_voice_mask`, `pending_restore_mask`)
  through `s32` views of `AkaoU32` members. This is not a type fact: a
  member store is `MEM_IN_STRUCT_P`, a scalar view is not, and the scalar
  view schedules the `key_off_request_mask` store after the branch as
  retail does (lev 7 as members).
- **`g_TextCursorStackTop` (`pe1/draw_state.h`, `int[]`)** is both the end
  of the text cursor stack (`menu/widget.c` compares cursor pointers against
  it) and the start of the wipe vertex scratch (`draw/wipe.c` views it as
  `u16` vertices and `DrawVertexPair`). Two records share the address; the
  casts go once the scratch area gets its own symbol or a union record.
- **Room class configure handlers (`room_lib/actor_classes.c`,
  `floor_walker_class.c`, `grab_attack.c`, room and scene class units)**:
  `(int *)arg0` / `(int *)arg3`. The opcode arguments are words whose type
  depends on the opcode (op 19 with a query writes results through them,
  op 25 stores a completion-signal pointer); the casts state the per-opcode
  reading of an `int` ABI slot.
- **Floor walker class object**: `RoomEnt`, `RoomFloorWalkerObject` and
  `RoomTransformScaleOwner` are three views of one class object; they agree
  up to `sub` at `+0x0C`, after which `RoomEnt.mat` (`+0x1C`) overlaps the
  walker's `leader` pointer. The views need a common class-object header
  with per-class tails before the `(RoomFloorWalkerObject *)o` casts can
  go.

## Link ownership

Splat's `undefined_syms_auto` and `undefined_funcs_auto` files are address/name
catalogues, not linker inputs. A linker-script assignment overrides a genuine
definition from an object and can make promoted C appear to link while its
symbol remains pinned. The build generates `undefined_addr_aliases.main.txt`
from the linked objects and emits only symbols that remain unresolved.

Tentative COMMON globals remain pinned until their BSS/SBSS placement is
represented by the manifest and linker script. This is explicit layout debt;
ordinary text/data definitions become object-owned automatically and cannot be
silently shadowed by the complete splat catalogues.

## Overlay symbol uniqueness (2026-10-13)

The 190 overlays share one load window and link on their own, so a global
name may be defined by several of them. A single-executable port needs every
global name to be unique per meaning. `tools/scripts/overlay_symbol_collisions.py`
(`make overlay-collisions`, ratchet `make overlay-collision-check`, baseline
`tools/overlay_collisions_baseline.json`, report
`docs/OVERLAY_SYMBOL_COLLISIONS.md`) reads the built overlay objects and sorts
every name defined in more than one overlay:

- `identical`: same section kind, size, bytes with relocated words masked and
  the same relocation targets. This is the shared source (`room_lib`,
  `fx_common`) compiled into each room: one definition can serve all.
- `conflicting`: different content under one name. Split by cause:
  *auto* (`func_`/`D_`/`__D_..._databin` names at an address several overlays
  use for different things; they disappear as the auto names get role names),
  *room interface* (a name a shared source refers to and every room defines:
  `g_RoomScript`, `g_RoomDrawList`, `RoomLib_SlotTable`, ...), *main* (a main
  symbol placed inside the overlay window, `g_StrFileDirArea`), and *private*
  (a named symbol used only by one overlay's own code).
- `overlay_vs_main`: an overlay global that `main.elf` defines too.

Rule: a symbol private to one overlay takes that overlay's prefix
(`docs/CONVENTIONS.md`, Overlays). The check fails on any private conflicting
name, on any new conflicting name and on any new overlay/main collision.
Room-interface names keep their spelling in the source, because renaming them
per overlay would mean editing the shared unit that names them; the
single-executable link gives each overlay's definition its own name instead
(`overlay_symbol_collisions.py --rename-map FILE` lists `overlay old new`
with `new = old__<overlay>`). Moving those names behind per-overlay macros is
a source change and is left until the port's link design needs it.

## Review contract

Each change must state which claim it makes: binary match, semantic recovery,
typing, naming, organization, or crutch removal. A green byte check proves the
binary claim only. It does not by itself prove the other claims.

## Shiftability

A goal of the reconstruction is that the code can change size without
breaking anything: adding, removing or growing a function must not invalidate
another address. A matching build proves the bytes; shiftability proves the
source does not depend on where those bytes sit.

Rules for source that aims at shiftability:

- Code and data refer to each other by symbol. A fixed address in C is allowed
  only for hardware (I/O registers, the scratchpad) and for the contract with
  the disc loader, such as the address an overlay is loaded at.
- Data that the program owns is defined in a translation unit and placed by
  the linker. Absolute symbol assignments in linker or symbol files are
  migration state, not a design.
- No constant that happens to equal a code or data address of this program:
  pointer-to-integer casts, integer-to-pointer casts of program addresses,
  address-named alias declarations and table entries typed as plain integers
  all count as hidden fixed pointers.
- Overlays link against the built main executable's symbols, not against
  addresses copied from a retail disassembly.
- Data produced outside the build (disc assets) that embeds program addresses
  is a documented ABI. Each such place is listed with its owner, and the
  addresses it needs stay pinned on purpose.

### Measuring it

`tools/scripts/shift_audit.py` (read-only, needs `make build` and, for the
overlay table, `make overlay-build-all`) prints the inventory below;
`--json` gives the same data. `tools/scripts/shift_test.py` is the
acceptance test. It relinks main from the built objects into a temporary
directory, with `--emit-relocs`, once as is and once with `--pad` bytes
(default 0x10) inserted at the start of `.main`, and compares every word of
the two images. A relocated word that moved by the pad is an explained
address field. A relocated word that did not move refers to an absolute
linker pin, a gp-relative offset that changed refers to a pinned small-data
symbol, and an unrelocated word shaped like a program address (data word,
`lui` upper half, `j`/`jal` target) is a literal address. The repository is
never written; the base relink must equal `build/USA/main.exe`.

The ratchet is `make shift-check` (`shift_audit.py --check`): it fails when
any counted category grows past `tools/shift_baseline.json`. It counts the
linked absolute assignments of main by category, every fixed-address column
of the C scan, the unrelocated words and absolute relocations of the
executable, the overlay pins and the hidden-address counts of the shift test
(pinned words, gp-relative offsets to pins, literal data words, `lui`
halves and jump targets, unexplained differences). Overlay pins are counted
as distinct addresses per overlay, so a second name for an address that is
already pinned is not growth; references into main are one total plus the
categories without main's own name. Unreferenced catalogue entries and
PROVIDEs that a definition overrides are not counted. `make verify` runs the
check after the build (the overlay counts are only complete once
`make overlay-build-all` has run), CI runs it with `SHIFT_REQUIRE_OVERLAYS=1`
after verifying every overlay, and `make shift-baseline` writes the lowered
baseline to commit with the change that lowered it.

### Inventory (2026-10-13)

Linked assignments for main, by what the address is (after wave 4a, see
[SHIFTABILITY_PLAN.md](SHIFTABILITY_PLAN.md#9-remnants-and-catalogue-wave-4a)):

| category | manual | generated aliases | remedy |
| --- | ---: | ---: | --- |
| hardware (SPU voice registers) | 10 | 0 | unavoidable |
| kernel/BIOS RAM (LIBCARD patch at 0xDFFC, 0xA000DFxx) | 2 | 1 | unavoidable (BIOS contract) |
| symbolic aliases (`a = b;`, record and type views) | 77 | 0 | not fixed; unify the C type where a byte-identical shape exists |
| anything else (pins of program code or data) | 0 | 0 | none left |

Every assignment in `linkers/USA/undefined_syms_manual.txt` sits in a block
whose comment starts with its kind (`hardware:`, `kernel:`, `alias`) and
names the owner; `shift_audit.py` groups the file by those blocks and counts
an assignment outside a block of its own kind as `unowned` (ratcheted at 0).
The 1224 names that nothing links against (address-form names from earlier
disassemblies, cheat-device and randomizer names, unused kernel vectors) are
in `configs/USA/symbol_catalogue.main.txt`, which is not linked;
`gen_expected.py` still reads it for the retail disassembly's labels. 319
names outside the 2 MiB RAM (misdecoded jump targets) were dropped.
`undefined_syms_auto.main.txt` (495) and `undefined_funcs_auto.main.txt`
(empty) are splat catalogues and are not linked either.

Fixed addresses in C (comments stripped): 10 alias declarations
(`asm("name")`) whose target is pinned somewhere (none in `src/main`, 6 in
`include`, 4 in overlays) and 6 integer-to-pointer casts of literal program
addresses, all in `src/main/draw`; there is no other literal program
address. The crutch ratchet's `pointer_integer_casts` regex matches 7 casts
in main, 11 in `include` and 74 in overlays; they do not bind an address.
What is left, and why:

- `Draw_InitGpuHwRegs` and `Draw_FlushPrimQueue` (`draw/gpu_regs.c`,
  `draw/prim_queue.c`) load the primitive queue block at
  0x80070E04..0x80070E50 (a 0x50-byte gap in `.text` after
  `Draw_PackPrimColor`) with `lui`/`ori`. No C form reaches those bytes with
  a symbol: GCC emits `la`, which assembles to `lui`/`addiu` (opcode 0x09,
  not 0x0D), and MASPSX expands `li` only for numeric operands. The proof
  and the consequence (a shifted `.text` moves the block but not these six
  literals) are in SHIFTABILITY_PLAN.md section 9; `shift_audit.py` lists
  them with that reason.
- Three `cdrom.h` views of `D_800B0E08`, `g_GameStateBlock`
  (`D_8009D1A0`) and `g_ActiveDrawSlot` (`D_8009CDDC`, also in the
  boot_display overlay) are second names that keep retail's reloads (see
  "Still open" above). In main they bind to the label the data blob
  defines; they count because overlays pin the same label. Giving the label
  the C name moves every overlay reference to a name main lacks until
  overlays link against main's symbols (step 3 below).
- fx_common's views of main data (`g_FxTextureSetup`, `g_FxCommonSceneIds`,
  `g_FxCommonGameFlags` on `g_FieldBgTextureSlots`, `D_800B0CD8`) have
  types that differ from main's declarations (signed fields, a record over a
  word), so they wait for the same step.
- `psyq_cd.h`'s halfword view of `g_CdStreamReady` (memory card overlay)
  is a type view of a named symbol, not an address.

Built main executable: 7756 relocations resolve to 502 absolute
program-range symbols (6490 address words and 1265 gp-relative offsets in
the shift test). 3 data words and 7 `lui` halves look like program addresses
without a relocation. None of the data words is an address: two are the
halfword pairs 0x7FFF/0x8001 of a table in `dtail_gp_post_dsfile`
(`0x80017FFF`) and one sits in a table of random-looking words
(`D_8009BE78+0x5C`). Of the `lui` halves, six are the register-block loads
above and one is the multiplier 0x80010003 of a division by 0x7FFF in
`Render_LoadFontGlyphAlt`.

The literals removed on 2026-10-13: the LIBCD BIOS descriptor in
`libcd/bios_4.c` points at `g_CdLastCom`, `g_CdStatus`, `g_CdLastPos`, the
event bytes `g_CdInterruptEvents`, `g_CdSyncResult` and the bios.c version
string; `DsReadCallback` stores
`g_CdReadCompleteCallback` by name; and six data words in the asm blobs
assemble as symbol references. `configs/USA/sym.main.txt` labels what they
point at (`g_StrFileDirArea`, `g_PmTaskProgramArea`,
`g_ErrorSeverityFatalName`, the LIBETC/LIBGPU/LIBCD version strings), and
`configs/USA/rel.main.txt` gives the memory card wildcard path as
`g_MemCardSaveSupportData + 0x10`. A data word in a splat blob takes a symbol
the same way: a `sym.main.txt` name at the target address, or a
`rel.main.txt` line (`rom:<file offset> reloc:MIPS_32 symbol:<name>
addend:<n>`) when the target is a field of a larger object. In overlays the
fx_common buffers past the end of its image, the boot display image buffers
and the memory card screen views are named in the overlay symbol files
instead of address aliases, and the MDEC wait loops pass their message
strings by symbol.

Overlays never see the built main executable. Each overlay links three
generated scripts (`undefined_funcs_auto`, `undefined_syms_auto` from splat,
`undefined_extra` from `overlay_extra_undefineds.py` using the overlay's
symbol file and `configs/USA/sym.main.txt`), all holding retail addresses.
Counted as distinct addresses per overlay, the 190 overlays pin 7349
addresses in main (389 distinct across all overlays): 6249 by the name main
uses, 711 that main names differently, 140 that main itself pins and 299
that main has no symbol for. They also pin 90 own definitions (splat's
cross-segment symbols), 114 own-window addresses no object defines, 375
addresses in other overlays' windows used as runtime buffers and 14 symbols
of other overlays. Since 2026-10-06 `overlay_extra_undefineds.py` reads only
the objects the overlay's linker script links and skips every name one of
them defines. The two overlays with copied foreign-VMA code
(`OVERLAY_VMA_OVERLAP`, `boot_display` and `menu_memcard`) keep those pins
(`--pin-defined`). The splat `undefined_*_auto` scripts still pin.
Overlays link against the built main executable (since 2026-10-13). Each
overlay links one generated script, `linkers/USA/overlays/link_symbols.<overlay>.txt`,
written by `tools/scripts/overlay_link_symbols.py` after main is built. For
every symbol the overlay's linked objects use and do not define it takes, by
name and in this order: the loader ABI list `linkers/USA/overlay_abi.txt`;
the address `build/USA/main.elf` gives the name now (never a main symbol
inside an overlay load window); the overlay's own window, hardware or kernel
from its address catalogue (splat's `undefined_*_auto` files and the overlay
symbol file, which are no longer linked). Anything else fails the build: a
main address that main does not name, or another overlay's memory that the
ABI list does not hold. `overlay-build-all` builds main first, and
`parallel_overlay_make.py` folds main's symbol table into each overlay's
cache fingerprint, so an overlay relinks whenever main moves a symbol.

The overlays' splat configs read `configs/USA/sym.main.txt` as a second
symbol file, so their disassembly names main's addresses the way main does
(`jal rand`, not `jal func_80071A54`). Splat reports every main entry as
outside the overlay's segments; that report goes to
`build/USA/overlays/<overlay>/splat.log`. The overlay symbol files no longer
carry main addresses, and the overlay C refers to main by main's names.

`overlay_abi.txt` lists the addresses that stay fixed on purpose, each block
with its owner and reason: the mdec_diag entry points other overlays call,
boot_display's work RAM outside its image, the memory card menu block reached
at its other link and load bases, and scene overlays' own memory past the end
of their extracted image inside the room load window. Section (d) of
`shift_audit.py` reads the linked scripts:

| (d) category | before (three scripts) | after (link_symbols) |
| --- | ---: | ---: |
| main_same_name (linked by name from main.elf) | 6247 | 7309 |
| main_other_name | 712 | 0 |
| main_no_symbol | 299 | 0 |
| main_pinned_in_main_too | 140 | 0 |
| overlay_window_buffer + other_overlay_symbol | 375 + 14 | - |
| abi_loader_contract (overlay_abi.txt, 435 entries) | - | 458 |
| own_overlay_pinned | 114 | 163 |
| shadows_own_definition | 89 | 0 |
| unreferenced | 5267 | 0 |

A 0x10 shift of main (`shift_test.py --scratch`, then each overlay's script
regenerated from the shifted ELF and relinked) moves all 7309 main-derived
assignments by 0x10, leaves the 458 ABI and 163 own-window ones in place,
and changes 35919 words in 189 of the 190 overlays, every one of them at a
relocation against a symbol that moved.

Fixed layout: `.main` at 0x80010000 (the PS-X EXE load address, a loader
contract) and the initial stack pointer 0x801FFFF0 (`PSX_EXE_STACK_TOP`). The
header's initial PC is `__SN_ENTRY_POINT` and its text size
`PSX_EXE_TEXT_SIZE`, both from the link (retail 0x80072534 and 0x1EE000;
the literal entry point started a shifted build in the middle of the code
before the startup, found by booting one in an emulator). Since
2026-10-13 `_gp` is `_ramsize` (the first `.sdata` word), `.field_engine`
follows `main_VRAM_END`, and the overlay window block (every overlay load
window and the runtime buffers in them) follows the boot work area as a
unit: the windows are offsets from its start
(`linkers/USA/overlay_windows.txt`), overlays link at those offsets from the
built main, and the link fails only when the image reaches the initial stack
pointer (0x1FF0 bytes of room). `tools/scripts/shift_full_test.py` (`make
shift-full-test`) relinks main and all 190 overlays with main shifted and
shows that every differing word is an address field moved by the shift.
It also lists overlay data words that hold one of the overlay's function
addresses without a relocation; none are left. The rooms' class tables
(seven methods per class) are C (`g_RoomClassTable`,
`room_lib/actor_class_table.c` and two window variants), and the room
script's pointer records and the other pointer-holding records are data
subsegments that name their target instead of databin bytes; each table
and record run is a row of [overlay_data_pointers.csv](overlay_data_pointers.csv)
(SHIFTABILITY_PLAN.md, emulator test, blocker 3).
The plan for main's data (phase 3: pins, BSS ownership, COMMON, ABI list and
the work list) is [SHIFTABILITY_PLAN.md](SHIFTABILITY_PLAN.md).

### Shift test result

With the script unchanged a shift does not link: `.field_engine` at its fixed
VMA overlaps the grown `.main`. With `.field_engine` and `_gp` rebased in the
scratch copy, a 0x10 shift gives 20568 explained address fields, 6490 words
still pointing at pinned symbols (388 symbols; most used `g_CurrentEntity`,
`g_GameState`, `g_AkaoCmd`, `g_AkaoCurTrack`, `_spu_RXX`), 1265
gp-relative offsets to pinned small data, 3 data words and 7 `lui` halves
that look like addresses (none of the words is one, see above), 28 hardware
references and no unexplained difference. The header fields do not follow
the shift. On 2026-10-05 the same run reported 5655 pinned words, 1491
gp-relative offsets, 21 literal data words and 17 literal `lui` halves.

### Removal order

1. Done: 931 manual assignments that duplicated a definition at the same
   address in a linked object. Byte-identical; they only hid the object's
   ownership.
2. Done (wave 4a): the unreferenced manual entries moved to the unlinked
   `configs/USA/symbol_catalogue.main.txt` (1224 kept for
   `gen_expected.py`'s label table, 319 out-of-RAM names dropped).
3. Overlays link against the built main symbols (`ld --just-symbols` on the
   main ELF, or generated assignments taken from `build/USA/main.map`), and
   `overlay_extra_undefineds.py` stops pinning names the overlay defines
   itself (done for `undefined_extra` outside the VMA-overlap overlays).
   Clears the 7349 pinned main addresses and the self-shadowing pins; needs a
   check for names that exist in both main and an overlay.
3. Done (2026-10-13): overlays link against the built main symbols by name
   (`overlay_link_symbols.py`), with the loader contract in
   `overlay_abi.txt`. The overlays' own-window catalogue pins (163) and
   their run-time memory listed in the ABI file are what remains fixed on
   the overlay side.
4. References that a linked object already names under another label (57
   function pins, about 400 data pins into asm blobs): give the blob label the
   C name through `sym.main.txt`, or change the C reference. Mechanical per
   symbol, byte-identical.
5. Alias declarations onto pins (10 left) and integer-to-pointer literals
   (6 left, the register block): replace by the owning symbol once steps 3,
   4 and 6 give it one.
6. BSS ownership: the zero blobs and the 111 COMMON pins become `.bss`/`.sbss`
   definitions in their units with a manifest placement. This is the large
   step (several hundred variables) and is what lets `_gp` and
   `.field_engine` be derived (`ADDR(.sdata) + 0x8000`, end of `.main`)
   instead of fixed.
7. Done except the register block: literal words and `lui` halves became
   symbol references (C, `sym.main.txt` labels, `rel.main.txt` relocations);
   the 10 address-shaped words left are the register block and four values
   that are not addresses.
8. Header: initial PC from the entry symbol, text size from the section end.

Unavoidable, by design: hardware registers and the scratchpad, the kernel and
BIOS addresses (LIBCARD patch, `0x80000000` vectors), the `.main` load address
and stack top, and the layout inside the overlay window block (the windows'
offsets), which the overlays prelinked in the retail `PE.IMG` depend on; the
block itself moves with main (SHIFTABILITY_PLAN.md section 6).

### Soft debt and remaining crutches in render, scene, sys, boot, cdrom, entity and memcard (2026-10-12, zero-debt batch 3)

Byte-pointer arithmetic, raw offset dereferences, pointer/integer casts and
field macros are gone from these modules (`Render_DrawObjectVariant` was out
of scope). Recipes, none of which changed a byte:

- Game-state fields: the per-slot ordering tables at 0x160 share their words
  with the background texture blob (`shared_buffers` union), the voice bank
  work bounds are `u8 *`, the fade primitives use a `Pe1PrimTag` byte view
  for the `len` byte, and 0x004 is `saved_state_004`. Work-buffer bases
  (`g_RenderFrontBufferBase` and the `g_SceneBankAssetSource..D_800B0E68` views) are
  byte pointers.
- A state pointer stepped by a word index (`Pe1GameStateAddress`), an
  allocation slot addressed by byte offset (`EntityAllocationAddress`), the
  collision database as the word added to region offsets
  (`CollisionDatabaseAddress`) and packet link words (`RenderLinkAddress`)
  replace `(u8 *)` sums and `(u32)` casts with the operand order retail has.
- `Boot_RunFrame` reaches the flags back from its scene flag byte with
  `GAME_STATE_OF_SCENE_FLAGS` (container-of on `bank_state_12`); retail
  computes `&g_GameState` as `scene_flags - 18`. The fade-to-white test reads
  the colour through `RenderTintTargetView`; the mode bit is a nested `if`,
  because one `&&` chain lets fold merge the blue and mode compares into one
  masked word load.
- `Draw_InitPipeline` clears `D_8009D2E8` through the symbol; the linker gives
  the same `lui 0x800a`/`-0x2D18` pair as the absolute address did.
- `Asset_ResetGameState` writes named fields; two loop counters (`i` for the
  scene object, `j` for the key/table loops) replace the `$3`/`$5` pins and
  two launders, and retail's store order of the movie fields is wide,
  active, frame, volume, index, sample bank.
- The menu tail in `Menu_MainUpdate` reads the player coordinates through a
  plain `s16 *` (not a structure member, so the `D_8009D28C` store keeps its
  place) from `var_t0`, now a register union of the m2c word and the entity
  pointer so the allocation is unchanged.
- PSY-Q DS/CD state, the save SJIS argument block, the CD mode bytes, the
  stream end state and the cursor text windows have one linker name each, so
  their `__asm__` labels are gone.

Left, with the best crutch-free lev:

- `Asset_ResetGameState`: the `$3` pin on the colour fill (lev 82: without it
  local-alloc puts the draw mode in `v1`, `a0` frees up for the table counter
  and the state base moves from `a2` to `a1`; no statement order of the flags
  read or fill form helps) and the `minusOne` launder (lev 9: the halfword
  and byte `-1` stores share one register).
- `Boot_InitMemoryLayout`: the `$2`/`$4` pins and the volatile buffer views
  (lev 60 without: sched1 reorders the stores to distinct symbols).
- `Entity_InitFreePool` keeps its byte-offset loop (now through the union)
  because the file is `-fno-strength-reduce`; dropping the marker reverses
  the loops (lev 8).
- Header aliases: the stuck pairs listed under "Still open" above, plus the
  overlay-shared `g_ActiveDrawSlot`/`g_GameStateBlock` labels. (The PSY-Q
  callback views were renamed in the 2026-10-12 soft debt sweep below.)
- Stack reserves: `Menu_DrawTextboxEntries` (its 0x38 slot) and the four
  battle phase/update functions (0x18 to 0x300 bytes); their original locals
  are unknown, and an unused named local would only rename the reserve.

### Soft debt sweep over src/main (2026-10-12, zero-debt round 4)

Byte-pointer arithmetic, raw offset dereferences and pointer/integer casts in
`src/main` went from 70/14/227 to 8/3/7, and every remaining site is either in
`fieldfx/geometry.c`/`Render_DrawObjectVariant` (out of scope) or
listed below. None of the recipes changed a byte.

Why most sites needed a union rather than a typed index: GCC 2.7.2 and 2.8.1
expand every C pointer sum (`p + i`, `&p[i]`, `i + p`) with the base first
(`addu d, base, offset`), while retail often has `addu d, offset, base`, the
order of an integer sum. Seven forms of typed indexing were tried in
`Battle_DrawDecimalNumber` alone; all give the base-first `addu`. The integer
sum therefore stays, but it is written on the `word` member of a typed
address union (the established `...Address` recipe), so no pointer is cast.

- Address unions added: `GteAtanEntryAddress` (ratan2), `StHeaderAddress`
  (StFreeRing, init_ring_status: the status is cleared as a plain halfword or
  word, because a member store lets GCC hoist the `StRingAddr` reload out of
  the loop), `CdTimeoutFlagAddress`, `CdDsQueueEntryAddress`,
  `DslNameAddress`, `DsSlotAddress` (DsSync's history reads),
  `GpuQueueEntryAddress` (_exeque), `CardObjAddress` (LIBPAD ports,
  capability records, multitap counts and the actuator flag cursor; pinned
  register unions keep the `$2`/`$3` pins where they were),
  `SpuReverbEntryAddress`, `TextboxGlyphSpacingAddress`,
  `RenderBufferSlotAddress` (frame-slot views of the buffer table and
  `g_OtBufferTable`), `BattleHalfwordAddress`, `BattleGaugePrimAddress`,
  `BattleSpriteWord` (the action-count sprites, whose m2c integers now hold
  an offset and then the packet address in one union) and
  `BattleInitSlotAddress`. A union is declared in the innermost block: at
  function scope `ratan2` got an 8-byte stack slot for it.
- Named idioms: `GTE_MULTU` (an unsigned 32x32 product read from LO; an
  inline function instead adds copies of the pinned operands),
  `GTE_SINE_WORD_AT_OFFSET` (the rotation builders keep the scaled angle in
  `$24`), `ISO_NEXT_DIRECTORY_RECORD`, `PSX_ADDRESS` (DMA address registers,
  ordering-table links and packet tags), `CDROM_EVENT_STATE_FROM_RETRY`,
  `RENDER_BUFFERS_FROM_GLYPH_SLOT`, `CARD_OBJ_FROM_STATUS_BUFFER` and PSY-Q's
  `va_start`/`va_arg` (`pe1/psyq_stdarg.h`).
- Types instead of casts: `dma_execute` takes `void *`; the stream payload is
  `StSectorPayload *`; `__fixdfsi` reads the sign through `MathS64`; the
  rotation builders' packed sine word is `s32`; `PadInitDirect` walks
  `u8 **` statusBuffer members; the DS callback setters take
  `DsEventCallback`/`DsCallback`.
- Asm-label aliases replaced by symbols (sym.main.txt or the manual registry):
  `atan_table`, the four VSync words, `g_CdReadyEvent`,
  `g_CdRomEventCommandState`, `g_CdRomCmdTimeout`, `g_CdRomCmdRetryState`,
  `g_CdReadProgress`, `g_CdStreamReady`, `g_DsReadCallbackCursor`, the three
  DS callbacks and the two DS cache words (now named where `dsfile.c` defines
  them). Two scalar views stay as `__asm__` aliases, now of the role name:
  `g_CdReadCommandPollToggle` (through `g_CdReadProgress.commandPollToggle`
  LIBDS_DSSYS_1_text_4A4 keeps the address in `$s0`; lev 18)
  and `g_CdStreamReadyHalfword` (through the union the memcard overlay
  schedules its test and clear differently and fails its hash).

Left, with the best crutch-free lev:

- `Battle_DrawStatusPanel` (miss sprite link, `(u8 *)D_800B01C8 - 8`): as a
  `BattleSpriteWord` integer sum the frame/panel offset multiplies swap
  `a1`/`a2` (lev 16); with a byte member the `-8` moves after the sum.
- `Draw_SetupSprites` emit macros: the `BattleSpriteWord` form of `EMIT_BASE`
  reorders the base load against the slot multiply (lev 76); see the earlier
  entry for the typed forms.
- `Menu_MainUpdate` `reset_operand` (mask, then `D_8009D278`): any union
  makes the actor load take `a0` instead of reusing the mask register `v1`
  (lev 8 over the unit). The `var_v0` tint/entity word is a register union.
- `CQ_vsync_system`: the queue window as a union word swaps `v0`/`v1` around
  the index multiply in every form tried (lev 4).
- Stack reserves and the `cdrom.h` sound-table aliases: unchanged, see above.
