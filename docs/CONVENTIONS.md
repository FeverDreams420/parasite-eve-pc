# Conventions

The working contract for source in this repository. It describes the tree as
it is and the direction it is moving in. Evidence rules for placing code are
in [CODE_ORGANIZATION.md](CODE_ORGANIZATION.md); the assembler policy is in
[ASM_AND_GTE_POLICY.md](ASM_AND_GTE_POLICY.md). Where this file and an older
file disagree on naming or layout, this file wins.

Two principles decide most questions:

- **Fidelity to evidence, not to the pain.** Keep what the binary proves
  (addresses, object boundaries, ABI, PsyQ names). Everything the binary does
  not prove is ours to name and organize for the reader.
- **Only PsyQ names are original.** Sony SDK functions, objects and libraries
  keep their SDK names. Every other name in the tree is a project name chosen
  by responsibility, and may be improved when understanding improves.

## Naming

| Kind | Form | Examples |
| --- | --- | --- |
| Game function | `Prefix_VerbObject`, PascalCase after a subsystem prefix | `Scene_LoadRoom`, `Inv_InsertItem`, `CdRom_BootReadFlow` |
| PsyQ function | exact SDK spelling | `SpuSetReverb`, `CdControl`, `RotMatrixYXZ`, `_card_auto` |
| Unknown function | `func_XXXXXXXX` (address, upper-case hex) | `func_80192F9C` |
| Global variable | `g_PascalCase` | `g_GameState`, `g_PlayTimeFrameCounter` |
| Unknown global | `D_XXXXXXXX` | `rcossin_tbl` |
| File-local (`static`) data or function | `s_PascalCase` data, `Prefix_Name` functions | |
| Type (struct, union, typedef) | PascalCase, no `_t`, no `struct` tag-only types | `Pe1GameState`, `FieldActor`, `AkaoTrack` |
| Struct member | `lowerCamelCase` for new code; existing `snake_case` members stay until the struct is reworked | `directoryOffset`, `pe_image_base_lba` |
| Enum constant / `#define` constant | `UPPER_SNAKE` with a subsystem prefix | `INV_CMD_GET_REMAINING_AMMO`, `STATE_FLAG_READY` |
| Macro | `UPPER_SNAKE` with a prefix that names its owner | `PE1_OFFSETOF`, `PSYQ_BIOS_TRAMPOLINE`, `BOOT_CALL_ON_SCRATCHPAD_STACK` |
| Header | `include/pe1/<subsystem>[_topic].h`, lower snake | `cdrom.h`, `menu_widget.h`, `akao/track.h` |
| C file (`src/main`) | lower snake case, the unit's role inside its subsystem directory, no directory or function prefix | `battle/encounter.c`, `item/actions.c`, `boot/main.c` |
| PsyQ C file | the SDK object name, SDK case | `psyq/libspu/s_ini.c`, `psyq/libcd/sys.c` |

Prefixes are fixed per subsystem (table below). Use the subsystem's prefix
even when the caller is elsewhere: a CD routine is `CdRom_*` wherever it is
called from. Game code does not use the legacy spellings `CD_`, `Cd_` or
`SPU_`; they became `CdRom_` and `Spu_`. SDK-internal names that look similar
(`CD_cw`, `CD_init`, `GPU_cw`, `_spu_init`) are PsyQ names and keep their
spelling, and so do the project names of private SDK routines inside
`psyq/` (`Gpu_DmaTimeoutCheck`, `Sys_SetIntrCallback`).

The prefix names the responsibility, the directory names the unit the
binary places the function in, so the two can differ: a script opcode
helper compiled into a battle unit is still `Script_*`, and the entity
helpers the script interpreter's units define are `Entity_*`. What must not
appear is a prefix that no live subsystem in the table owns (`Task_`,
`FieldEng_`, `Tbl_`, `Sys_`, `Util_`, ...) or no prefix at all on an
external function. `tools/reorg/rename_symbols.py --audit` checks this for
`src/main` outside `psyq/` (`-v` lists the cross-subsystem functions per
directory), and `rename_symbols.py OLD NEW` or `--map FILE` renames a
symbol everywhere: symbol files, manifests, sources, docs and tests.
The same goes for the other names: a type, `g_` global, header or test
does not start with an old module's name either (`TaskNode`,
`g_FieldEngineSlotTable`, `task_node.h`, `test_task_run_queue.py` were
renamed to `ScriptNode`, `g_FieldFxSlotTable`, `script_node.h`,
`test_script_run_queue.py`). `--audit` checks these too (`--names` alone)
and lists the documented exceptions (GTE and GPU hardware types, the libgpu
headers) with `-v`; `rename_symbols.py --header OLD NEW` renames a header
with its include guard.

**Game file names.** A unit in `src/main/<subsystem>/` outside `psyq/` is
named the way its developers would have named a C file: lower snake case,
short, by the role of the unit inside its directory, unique there. The
directory already names the subsystem, so the file name repeats neither it
nor the function prefix (`battle/encounter.c`, not
`battle/Battle_EncounterFlow.c` or `battle/battle_encounter.c`), and filler
words (`Flow`, `Lifecycle`, `And`, `Driver`) are dropped
(`akao/voice_bank.c`, `fieldfx/geometry.c`, `menu/status_panels.c`). A
unit named after the one function it is known for keeps that function's
plain name (`boot/main.c`). Function names keep their `Prefix_VerbObject`
form; no file has to match a function. A unit whose functions belong to
another subsystem than its neighbours' lives in its own subsystem's
directory (`menu/main_update.c` lies between field and battle code).
`make organization-check` counts names that break these rules
(`unconventional_files`, ratcheted at zero). Overlay units keep their
current names (`RoomLib_ActorClasses.c`), except the two shared-code
overlays `boot_display` and `menu_memcard`, whose units and templates
follow the same lower snake-case role rule (`boot_display/text_screen.c`,
`menu_memcard/video_playback.c`, `menu_memcard/video_open.inc`). The
function-style names the `src/main` units had before are listed with their
new paths in `tools/reorg/maps/step9_snake_case_names.csv`, those of the
two overlays in `tools/reorg/maps/step9_overlay_shared_units.csv`.
(`unconventional_files`, ratcheted at zero). The shared room library
`src/overlays/room_lib/` follows the same rule (`orbit_flare.c`,
`class_spawn.c`, counted in the overlays scope); the other overlay units
keep their current names. The function-style names the `src/main` units
had before are listed with their new paths in
`tools/reorg/maps/step9_snake_case_names.csv`, those of `room_lib` in
`tools/reorg/maps/step10_room_lib_units.csv`.

**PsyQ file names.** A unit under `psyq/<library>/` is named after the SDK
object it reproduces, lower case (`INTR_VB` is `intr_vb.c`, the BIOS veneer
object `A50` is `a50.c`, `SSINIT_C` is `ssinit_c.c`). The object comes from
`configs/USA/psyq_provenance.json`; a 12-byte BIOS veneer that has no entry
there is named after the PsyQ signature object whose bytes it matches
exactly. When one object is still split over several units, because the
pieces need different compiler settings or carry declarations that do not
yet agree, the units are numbered in address order (`padcmd.c`,
`padcmd_2.c`, ...) and merged once they compile together. Every unit opens
with a one-line comment naming the library, the object (and part) and the
public symbols it defines. Functions and globals take the object's SDK
labels; private routines that only have generated `text_*` labels keep
project names.

`MEMMOVE` and `QSORT` are byte-identical in LIBC and LIBC2. The executable
links LIBC: its string and memory routines are the LIBC BIOS veneers
(`C21` strcat, `C42` memcpy, ...), not LIBC2's C bodies. Both objects
therefore live in `psyq/libc/`; there is no `psyq/libc2/`.

Placeholder names are migration state, not style: `func_`/`D_` symbols and
files such as `misc19.c`, `task5.c`, `cd_rom.c` or `gap_*` that describe
position rather than responsibility. `make organization-check` ratchets their count; it
may go down, never up. A file name that ends in an address
(`RoomEffect_JointBeacon_8018FDC4.c`) is allowed only to tell apart several
instances of the same template inside one overlay.

Name by what the code does for the game (`Save_StartWriteSlot`), not by how
it does it (`Save_WriteBytesLoop`) and not by where it was found
(`Main_Func3`). Do not encode types, sizes or addresses in names that have
been identified.

## Subsystems in `src/main`

Each directory is a subsystem with one responsibility and one function
prefix. "Target" is where the files belong; moving them is a separate change
that keeps `make check` green and updates the manifest paths. Do not create a
directory for one file.

**PsyQ code under project names.** About 120 game-named units outside
`psyq/` lie inside SDK object ranges recorded in
`configs/USA/psyq_provenance.json`: about half of `draw` (LIBGPU SYS and primitive
objects), the `CardObj_*`
files in `memcard` (LIBPAD/LIBCARD), most of `pad` (LIBAPI PAD/PATCH), the
64-bit helpers in `math` (LIBMATH), the `Spu_*` setters in `akao` (LIBSPU),
and `gte` (LIBGTE). They move to `psyq/<library>/` under their SDK names.
Confirm each with the provenance entry first: a unit can straddle an object
edge, as `akao/spu_transfer.c` did before LIBPAD WAITRC2's
`chkRC2wait` was split off it.

| Directory | Prefix | Responsibility | Target |
| --- | --- | --- | --- |
| `psyq/lib*` | SDK names | Sony PsyQ 4.0 libraries, one directory per SDK library | keep; receives the units above |
| `boot` | `Boot_` | `main`, start-up, subsystem init, the per-frame loop | keep |
| `sys` | `Sys_` | interrupt and BIOS glue, shutdown and reset | dissolved: `Draw_ReportError` is in `draw`, `asset/game_state.c` (with `Boot_Shutdown` and `Boot_FinishFrame`) in `asset`; SDK parts and `setjmp.c` (LIBAPI veneer) belong in `psyq`, where private LIBETC/LIBAPI routines keep their `Sys_` project names |
| `cdrom` | `CdRom_` | game-side CD layer: sector reads from PE.IMG, retries, boot reads | keep; `cd_rom*.c`, `misc9.c` renamed by content |
| `memcard` | `MemCard_` | memory-card access for saves | keep the game part; LIBPAD/LIBCARD units and BIOS veneers (`_card_*`, `InitCARD2`...) to `psyq` |
| `pad` | `Pad_` | controller input | mostly LIBAPI: move to `psyq/libapi`; what remains joins `boot` or `menu` input |
| `draw` | `Draw_` | game 2D drawing: text, glyphs, wipes, number and bar widgets, packet pools | keep the game part (renamed from `gpu`, prefix from `Gpu_`); LIBGPU units to `psyq/libgpu`, where private SYS routines keep their `Gpu_` project names; `Scene_*` files to `scene` |
| `gte` | `Gte_` | none of its own | dissolved: the LIBGTE objects are `psyq/libgte/<object>.c` under SDK names; the boot-time light and table setters are `Boot_*` |
| `math` | `Math_` | fixed-point and integer helpers | keep; the LIBMATH objects are `psyq/libmath/<object>.c` under SDK names |
| `render` | `Render_` | 3D object, room and sprite rendering, fades, camera | keep; `Scene_*Battle*` predicates to `scene` |
| `anim` | `Anim_` | skeletal animation decode and interpolation | keep |
| `entity` | `Entity_` | actor pools, movement, hierarchy, per-frame actor update | keep; `task.c` renamed by content |
| `obj` | `Obj_` | geometry-state entry slots | dissolved: its functions are `Render_*Entry*` in `render` |
| `field` | `Field_`, `Geo_` | field map entries, collision and floor geometry | keep; the mesh-list `Geo_*` in `render` became `Render_*` |
| `fieldfx` | `FieldFx_`; module classes `GlowEffect_`, `GlowEffectB_`, `GlowEffectC_`, `FourLayerGlow_`, `PointSprite_`, `BurstEffect_`, `BurstEffectB_`, `BurstEffectC_` | the field effect engine in the separate `0x800C20C8` segment: effect pool, emitters, particles; each of the eight effect modules names its seven class methods `<Module>_<Method>` (pe1/field_effect_modules.h) | keep (renamed from `engine`, prefixes from `FieldEng_` and `FieldAnim_`) |
| `scene` | `Scene_` | room and scene loading, story flags, scene dispatch | keep; receives `Scene_*` files from `draw`, `main`, `render`, `time` |
| `overlay` | `Overlay_` | overlay tables and loading, overlay audio/texture slots | dissolved: the audio-slot functions (`CdRom_RegisterAudioSlot`, ...) and `CdRom_LoadInitialImage` are in `cdrom`, `Boot_LoadOverlayTables` in `pm`, `Scene_StreamTexturePage` in `scene` |
| `asset` | `Asset_` | PE.IMG asset tables, TIM upload | keep |
| `script` | `Script_` | actor script interpreter: opcodes, expressions, node pool | keep (renamed from `task`, prefix from `Task_`); the former `Task_Gpu*` units are `Draw_*` in `draw` |
| `pm` | `Pm_` | process manager: slot table that runs script batches | keep |
| `event` | `Evt_` | game event delivery and deferred execution | dissolved: `Menu_DeferredExec` is in `menu`; the old `Evt_Deliver` was LIBMATH FERR `_err_math` (`psyq/libmath/ferr.c`) |
| `battle` | `Battle_`, `BattleCmd_` | ATB battle: turns, targeting, damage, enemy AI, status display | keep; `Inv_Build*List.c` to `item`; `Math_IntSqrt` shares the battle command unit with the ammo commit (`battle/commands.c`) |
| `aya` | `Aya_` | the player character: stats, level table, parasite-energy spells | keep; `misc7.c` to `battle`, `Draw_LookupGlyphMetrics` to `draw` |
| `item` | `Inv_`, `Item_` | inventory, item table, equipment and its modifiers | keep; `Menu_*` files to `menu` |
| `menu` | `Menu_`, `MenuWidget_`, `MenuInput_` | in-game menus, widget tree, menu input queue | keep; `Inv_*`, `Save_*`, `Pad_*` files out to their owners |
| `save` | `Save_` | save-data layout, metadata, write flow | keep |
| `akao` | `Akao_`, `AkaoSpuVoice_`, `Seq_`, `SeqOp_`, `Spu_` | Square's AKAO sound driver: sequencer (`SeqOp_*` are the opcode handlers), voices (`AkaoSpuVoice_*` the per-voice SPU register writers), SPU uploads | keep; LIBSPU units to `psyq/libspu` |
| `audio` | `Sfx_` | misnamed: both files are menu item-list code | dissolved: the item-list functions are `Menu_*` (`Menu_ContextPick`, `Menu_DrawSlotRow`, ...) |
| `time` | `GameTime_` | play-time counters and timers | keep |
| `text` | `Text_` | text and textbox lookup tables, map-name and base-32 id strings | keep (renamed from `table`, prefixes from `Tbl_` and `Str_`); the textbox page helpers are `Draw_*` |
| `util` | `Util_` | vague | dissolved: `util.c` is `item/active_list.c`; its helpers took their owners' prefixes (`Text_CopyFFTerminatedBytes`, `Akao_CopyWords`, `GameTime_PackTime`) |
| `main` | mixed | splat's default bucket, not a subsystem | dissolved: the battle units are in `battle`, `MapSelectionFlow` is in `script` |

## Overlays

`src/overlays/<binary>/` holds the code of one overlay binary; the directory
name is the configured overlay name.

- **Rooms:** `room_mNNN` where `NNN` is the map number used by
  `Scene_LoadRoom`. Room overlays keep their numbers and gain a place-name
  suffix, for example `room_m005_carnegie_hall`. The suffix is lower snake case from the
  room's text bank (the `text-bank:` comment in its config). Renaming is a
  single change across config, `src/overlays`, symbol file and CI. The
  former `+1`/`+2` tails (`room_m126+1`) recorded an extraction delta, not an
  identity, and are gone; only the retail asset names keep them.
- **Scene sets:** `scene_eNN`, entry `NN` of the special-scene table at
  `0x80094048`, with the place suffix of their map the same way
  (`scene_e22_liberty`; docs/ROOMS.md).
- **Shared overlays:** `fx_common`, `fx_field`, `menu_memcard`,
  `boot_display`, `mdec_diag` (MDEC diagnostics and the PsyQ LIBPRESS VLC
  decoder, formerly `sys_reset`): named by responsibility.
  `fx_common_sector17` (formerly `render_clip`) is a data-only duplicate of
  that `fx_common` sector, named after the data it holds.
- **Shared overlay code:** `src/overlays/room_lib/` holds the units that
  several overlays link (the room library `actor_classes.c`, the effect
  modules such as `orbit_flare.c`, the module class methods `class_*.c`
  and the scene script entries `script_*.c`), which every overlay that
  links one lists in its manifest as `../room_lib/<unit>` and compiles into
  its own build, and the templates that several units still instantiate
  (`bounce_render.inc`, `fan_sweep_spark.inc`, `poll_reset_actor.inc`,
  with `room_lib.h`, `motion_particles.h` and `render_layouts.h`). A
  template with a single instance is written out in its unit instead.
- **Function names inside overlays:** `RoomLib_` for `room_lib` templates
  and the hooks they call (`RoomLib_GrabOnApproach`), `RoomEffect_` for
  effect templates (the former `RoomFx_` spelling is gone),
  `RoomMNNN_`/`SceneENN_` for code that belongs to one overlay, and the
  overlay's own prefix for the shared overlays: `FxCommon_`, `FxField_`,
  `MenuCard_`, `BootDisplay_`, `MdecDiag_`. Resident prefixes (`Boot_`,
  `Scene_`, `MemCard_`) do not appear on overlay functions; an overlay
  symbol file may still name a word of the main image it reads, and then it
  uses the main image's name (`g_PlayerEntity`); the `RoomMain_ActorPtr`,
  `RoomMain_ActorPtr2` and `RoomMain_RotTable` aliases (of
  `g_PlayerEntity` and the sine table at `0x800966EC`) are the remaining
  exceptions, left until their overlay declarations agree with the main
  image's types. Template parameter macros take the template's
  prefix in upper case (`ROOMLIB_`, `ROOMEFFECT_`, `MENUCARD_`).
- **Global names stay unique across overlays:** the overlays link into one
  executable in the port, so a global that only one overlay's own code uses
  carries that overlay's prefix: `RoomMNNN_`/`SceneENN_` for functions and
  `g_RoomMNNN`/`g_SceneENN` for data (`g_RoomM318ScriptArg2`,
  `g_SceneE08OrbScript`), never a bare `g_Room...` or an address name that
  another overlay also defines for different content. The exceptions are the
  room interface (a name a shared source in `src/overlays/room_lib` or
  `include/pe1` refers to, `g_RoomScript`, `g_RoomDrawList`, ...), which every
  room defines under the one spelling, and symbols whose content is
  identical in every overlay. `make overlay-collision-check` enforces this
  (`docs/OVERLAY_SYMBOL_COLLISIONS.md`, `docs/CODE_ORGANIZATION.md`).

## Headers

- `include/pe1/` holds the game's shared declarations; `include/pe1/akao/` the
  sound driver; PsyQ declarations live in `psyq_*.h`.
- One owner per declaration. A function prototype, global or type is
  declared in exactly one header: the header of the subsystem that defines
  it. Never repeat a prototype in a `.c` file; `make debt-check` forbids new
  file-scope `extern`s.
- No per-function headers. A header covers a subsystem or a coherent part of
  one (`menu_widget.h`, `field_effect_pool.h`). Existing one-function headers
  are folded into their subsystem header when touched.
- Module-private declarations shared by a directory live next to it
  (`src/overlays/room_lib/room_lib.h`, `src/main/psyq/libcd/bios_internal.h`).
  Declarations used by one file stay `static` in that file.
- Headers include what they use, have an include guard
  `PE1_<NAME>_H`, and contain no definitions that allocate storage. Every
  header compiles on its own: a private `*_externs.h` includes the headers
  that declare its types, and a unit-local type it names lives in it.
- Moving a declaration must keep its exact width, signedness, qualifiers and
  prototype. Do not "fix" an uncertain ABI while centralizing it.

### Includes

All includes are spelled with quotes; angle brackets are not used. A file has
one include block at the top, after its marker block and file comment (in a
header, after the include guard) and before any declaration:

```c
#include "common.h"
#include "pe1/gte.h"
#include "pe1/psyq_gpu.h"
#include "pe1/room_fx.h"
#include "m2c_macros.h"

#include "../room_lib/room_lib.h"
#include "m273_externs.h"
```

- `common.h` first, then the other include-path headers alphabetically: the
  `pe1/` headers (the PsyQ `pe1/psyq_*.h` sort among them), then
  `include_asm.h` and `m2c_macros.h`. A blank line, then the local headers
  next to the file, alphabetically, by their path relative to it.
- A header is spelled by its canonical path (`pe1/gte.h`, never
  `../../../include/pe1/gte.h`) and included once per file.
- A `.c` does not include a header after code. When the build proves an
  include has to follow a declaration, it stays there under a one-line
  `/* Late include: <why> */` comment. Two today: `boot/frame_loop.c`
  includes `pe1/boot.h` after its COMMON words, whose small-data decision the
  header's incomplete `D_8009CDA4[]` would otherwise change, and
  `psyq/libpad/padcmd.c` includes `padcmd_externs_1.h` after
  `_padLoadActInfo`, which calls the pointer the header prototypes without
  arguments.
- `.inc` files are function templates expanded in place after the `#define`s
  that configure them; they may appear anywhere and more than once.
- An `extern` or prototype is not repeated in a file, nor repeated from a
  header the file includes.

`make source-policy-check` runs `tools/scripts/check_includes.py`, which
enforces the block, the order, the spelling, the duplicates and the include
guards; `--fix` hoists, deduplicates and sorts (the build decides whether the
result is byte-identical).

## Assembler

- All code is C. There are no `.s` files in `src/`, and none may be added;
  `make source-policy-check` rejects them.
- Inline assembler appears only through evidenced macros: GTE operations
  (`include/pe1/gte.h`), the scratchpad stack switch
  (`boot_stack.h`), PsyQ assembler objects (`PSYQ_ASM_FUNCTION`,
  `PSYQ_BIOS_TRAMPOLINE`) and proven game assembler (`GAME_ASM_FUNCTION`).
- Every whole-function use is listed with address, size and evidence in
  `configs/USA/original_asm_evidence.json` (game) or
  `configs/USA/psyq_provenance.json` (SDK). No entry, no assembler.
- Register pins and empty barriers are compiler constraints, not
  instructions. They are allowed, counted by `make debt`, and removed whenever
  the match survives without them (`make drop-pins`, `make drop-barriers`).
  The `PE1_NOP*` scheduling slots (`psyq_nop.h`) are an authorized exception
  and are not counted. They reproduce handwritten LIBGTE FGO delay slots
  that stock GCC does not emit.
- The compiler and MASPSX are stock. Per-file options are one marker block
  at the very top of the file, one marker per line in this order, then a
  blank line:

  ```c
  /* GCC_VERSION: 2.8.1 */
  /* CC1_FLAGS: -G8 -g3 -fno-schedule-insns */
  /* ASSEMBLER: GNU */
  /* MASPSX_FLAGS: -G8 --use-comm-section --no-expand-div */
  /* MASPSX_FORCE_G0: 1 */
  /* ASPSX_VERSION: 2.56 */
  ```

  `-G<n>` comes first in its marker, the other options in the order of
  `tools/scripts/check_flag_markers.py`; defaults are not written out, and
  marker text appears nowhere else in the file (cc.sh greps the whole file).
  The defaults are cc1 `-G0`, assembler `-G0`, MASPSX `--expand-div` and
  ASPSX 2.70; a unit that needs the opposite says `--no-expand-div`
  (`math/fixed_div.c`) or `ASPSX_VERSION: 2.56` (`battle/encounter.c`).
  Small data uses the K8 set (cc1 `-G8`, maspsx
  `-G8 --use-comm-section`, `MASPSX_FORCE_G0`) or K0 (the same without the
  cc1 marker), with one COMMON definition per word retail reaches through
  `$gp`. `make source-policy-check` enforces the block (`--fix` rewrites
  it); docs/COMPILATION_FLAGS.md says what every flag does, which units
  need it and why.

### PsyQ marker sets

A `psyq/` unit carries only the markers its bytes need: each marker was
dropped where the unit compiles to the same object without it. SDK
assembler objects (`PSYQ_ASM_FUNCTION`) always keep `ASSEMBLER: GNU`
(`make source-policy-check`). The distinct sets left, per library, with
their unit counts (2026-10-12; "2.8.1" is `GCC_VERSION: 2.8.1`, "GNU" is
`ASSEMBLER: GNU`, "gas 2.7"/"gas 2.8.1" is `GAS_VERSION`, "nsa" is
`-mno-split-addresses`):

| library | units | marker sets (units) |
|---|---|---|
| libapi | 41 | none (35); GNU (5); 2.8.1 nsa (1) |
| libc | 18 | none (16); 2.8.1 (1); gas 2.8.1, 2.8.1 nsa (1) |
| libcard | 15 | none (10); GNU (4); 2.8.1 (1) |
| libcd | 20 | GNU (6); none (3); GNU 2.8.1 `-fno-expensive-optimizations` nsa (2); GNU 2.8.1 nsa (2); 2.8.1 (1); 2.8.1 `-fcall-used-$1` (1); 2.8.1 nsa (1); GNU 2.8.1 `-fno-schedule-insns` nsa (1); GNU `-fno-expensive-optimizations` (1); GNU `-fno-schedule-insns` `-fno-schedule-insns2` (1); gas 2.7 (1) |
| libds | 24 | GNU (12); GNU 2.8.1 nsa (5); none (3); 2.8.1 nsa (2); 2.8.1 `-fno-schedule-insns` nsa (1); GNU 2.8.1 `-fno-schedule-insns2` nsa (1) |
| libetc | 6 | GNU (2); GNU 2.8.1 nsa (2); GNU `-fno-cse-follow-jumps` (1); none (1) |
| libgpu | 28 | none (20); GNU 2.8.1 nsa (3); 2.8.1 nsa (2); 2.8.1 (1); GNU (1); `-O1` (1) |
| libgte | 39 | none (26); GNU (11); GNU gas 2.7 (1); gas 2.8.1 (1) |
| libmath | 10 | 2.8.1 (5); GNU (4); 2.8.1 `-g3` (1) |
| libpad | 7 | GNU (6); GNU `-fno-cse-follow-jumps` (1) |
| libsn | 1 | GNU (1) |
| libsnd | 1 | none (1) |
| libspu | 26 | none (11); 2.8.1 nsa (8); GNU (6); gas 2.8.1 (1) |

The spread is a toolchain limit, not a property of the SDK sources: PsyQ
built every library with one compiler, which the stock GCC 2.7.2 and 2.8.1
each reproduce only for part of the functions. Objects whose parts need
different sets stay split; see CODE_ORGANIZATION.md, "PsyQ objects split
over several units".

## Shiftability

Code must be able to change size without breaking anything. The full rule set
is in [CODE_ORGANIZATION.md](CODE_ORGANIZATION.md#shiftability); in practice:

- Refer to code and data by symbol. Never write a program address in C: no
  `(T *)0x800XXXXX`, no integer that happens to equal an address, no table of
  addresses typed as integers.
- Fixed addresses are allowed only for hardware: I/O registers and the
  scratchpad, each behind a named macro (`FIELDFX_SCRATCH`,
  `BOOT_SCRATCHPAD_STACK_TOP`), and for the loader contract such as an
  overlay's load address.
- Data the program owns is defined in a translation unit. Absolute
  assignments in `linkers/USA/undefined_syms_manual.txt` and in the splat
  symbol files are migration state; each one removed is progress.
- Two views of the same data are two typed declarations of one symbol, not
  a second address.
- Disc data that embeds program addresses is a documented ABI: list it with
  its owner and keep those addresses pinned on purpose.

## Text-resident data

Data, tables and zero padding that sit inside code segments are typed as what
they are. Tables become `rodata` (or `data` for written words) subsegments in
text order (`linker_section_order: .text`, section `.code_data`); zero words
after `jr $ra` and its delay slot become `pad`. Neither earns code credit.

## Code style

- Four-space indentation, no tabs. Opening brace on the same line as the
  function or statement; `else` on the closing brace's line.
- Aim for 80 columns, hard limit 100. Break long argument lists one level
  deeper than the call.
- One declaration per line for locals that carry meaning; declare at the top
  of the block (C89, as GCC 2.7.2 requires).
- Use the project types from `common.h` (`u8`..`s32`) for fixed-width data,
  plain `int` for counters and indices.
- Use struct members, not byte offsets. `M2C_FIELD`, `RW8`-style accessors and
  raw `+ 0x1C` arithmetic are migration state.
- Comments say why: what the code is for in the game, which evidence fixes a
  boundary, why an odd construct is needed for the match. Do not narrate the
  code, record history ("was previously..."), or paste scores and run logs;
  those belong in commit messages.
- A construct that exists only to reproduce the original code generation
  (volatile view, unused local that keeps a stack slot, `goto` retry loop)
  gets a one-line comment saying so.

## A finished file

A file is done when all of these hold:

1. Its object matches retail inside a byte-identical linked binary.
2. Its name and every function and global in it have project or SDK names;
   no `func_`/`D_` remains for anything it defines.
3. It declares nothing at file scope that belongs to another unit; it
   includes the subsystem headers it needs and nothing else.
4. It uses typed structures and enums instead of offsets and magic numbers.
5. It contains no program addresses (shiftability), no unrecorded pins or
   barriers, and assembler only through evidenced macros.
6. It sits in the directory of its subsystem, and its comments explain the
   non-obvious.

A file that meets 1 but not the rest is matched, not finished. The
organization and debt ratchets measure the distance.
