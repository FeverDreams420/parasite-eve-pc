# Source-tree reorganisation plan (src/main + src/overlays)

Measured on `main` @ 1acf4c9f8 (2026-10-12), read-only. Every number below
comes from the manifests, `build/USA/main.{elf,map}` (fresh build of 12:38),
`tools/scripts/main_tu_evidence.py` and a marker probe that compiled scratch
copies of every unit with `tools/scripts/cc.sh`. The probe compiled about 1,400
objects in the scratchpad. Its baseline objects match `build/` for 327 of 335
probed main units and 200 of 200 checked overlay units. The other 8 are
-g3 or line-sensitive units, which the probe cannot judge.

Working files (scripts + raw data), all in
`scratchpad/reorg/`: `inv.py` (main inventory → `rows.json`), `bounds.py`
(gap evidence per boundary), `probe.py`/`probe2.py` (marker probe →
`probe.json`), `groups.py` (module groups → `groups.json`), `mkmap.py`
(→ `mapping.md`, `mapping.csv`), `ovinv.py`/`ovprobe.py` (overlays →
`ov.json`, `ovruns.json`, `ovprobe.json`), `main_tu_evidence.json`.

---

## 1. Inventory

### 1.1 Resident executable, game code (`src/main` without `psyq/`)

| | value |
|---|---|
| C units (manifest `c` entries = files) | **339** in 26 directories |
| functions (definitions found in source) | ~1,950 (nm/evidence count: ~1,810 excluding PsyQ) |
| single-function files | **116** (34%); 2-function 62; 3-function 39 |
| zero-C-function files (GAME_ASM / GNU-as units) | 5: `math/Math_FixedMul`, `math/math_fixed`, `math/Math_SqrtApprox3`, `task/Task_GpuPackPrimColor`, `engine/FieldEng_SpinningModel` (template `.inc`) |
| `func_`/`D_` **file names** | **0** (organization baseline main: 0/0) |
| `func_XXXXXXXX` text symbols in main.elf | 271 of ~2,355 functions (11.5%), 36 files in `engine/` define most of them |
| distinct `D_XXXXXXXX` names referenced by game sources | 758 |
| distinct per-file marker signatures | **44** (CC1_FLAGS / MASPSX_FLAGS / FORCE_G0 / ASPSX_VERSION / ASSEMBLER) |
| suffix families | `engine/FieldEng_CollisionQueries{,_2,_3}`, `FieldEng_BurstEffect{,_2,_3,B,C}`, `FieldEng_GlowEffect{,B,C}` |
| directories with ≤2 files | anim 1, event 1, memcard 1, overlay 1, table 1, cdrom 2, pm 2, sys 2, time 2 |
| files whose prefix ≠ directory | 163 would change directory under the plan (e.g. 14 in `main/`, `battle/Inv_*`, `menu/Pad_*`, `boot/Gpu_*`, `field/Entity_*`) |

Per directory (files / functions / single-function files):
akao 6/273/2 · anim 1/3/0 · asset 4/24/0 · aya 4/7/2 · battle 47/89/24 ·
boot 6/14/2 · cdrom 2/10/0 · engine 41/343/3 · entity 15/50/5 · event 1/1/1 ·
field 3/14/0 · gpu 9/75/1 · item 29/172/7 · main 14/25/8 · math 4/1/1 ·
memcard 1/50/0 · menu 77/390/31 · overlay 1/3/0 · pm 2/18/0 · render 16/102/5 ·
save 10/23/7 · scene 8/18/5 · sys 2/9/1 · table 1/17/0 · task 33/211/11 · time 2/7/0.

**The binary-evidence picture** (`main_tu_evidence.py`, join threshold 1.0):
there are 1,623 evidence groups, of which 167 are multi-function and 1,456
singletons. Only 24 groups span several current files (111 files), and 23 of
them are mechanically mergeable. For most of the 274 boundaries inside the
proposed modules, the evidence is *neutral*: neither join nor split. Hard
split evidence (gprofile/compiler/sdk) appears at only 5 game boundaries, and
only one falls inside a proposed module (`boot/boot.c`, gprofile between
`Boot_RenderFlagsAndPadInput` and `Gpu_InitPipeline`).

**Markers are the real constraint.** Adjacent units alternate between -G0 and
-G8 throughout `task/` (m0/m5/m0/m5…). The probe compiled each unit under its
neighbours' marker sets and under 6 canonical sets. It counts a unit as
compatible with a set when every loadable section, relocation and symbol is
identical. Results:

- Inside the proposed modules, 200 of 274 internal boundaries are already
  pairwise marker-compatible.
- **74 are not.** By difference: cc1 -G 43, assembler -G 37, maspsx options
  34, cc1 -f options 22, FORCE_G0 21, ASPSX_VERSION 6, GNU-as 5.
- The probe also found a stale manifest comment. `FieldEng_CollisionQueries_2/_3`
  are said to need `-fno-schedule-insns`, but today `_2` and `_3` both compile
  byte-identically under `_1`'s `-fno-strength-reduce`. The three-way split is
  no longer needed.

**Data constraint.** 43 game units own a `.rodata` subsegment, and 31
unowned `rodata` blobs (`main/rodata_XXXXXX`) sit between them. Three proposed
modules have such a blob between members:

- `rodata_0009AC` in `script_actor_ops`
- `rodata_001324` in `battle_context`
- `rodata_0015FC` in `battle_hud`

Each blob must be claimed as `const` data in the module before the merge can
be byte-identical.

### 1.2 Resident PsyQ (`src/main/psyq`, out of scope for moves; layout note)

There are 268 files in 13 library directories, already named after SDK objects
(`psyq/<lib>/<obj>.c`); 79 of them are `_N` split pieces. Mapping unit .text
ranges onto `configs/USA/psyq_provenance.json` gives:

- 192 units fall in **134 distinct objects**; 76 have no provenance entry
  (data-only or veneers).
- **15 objects are split over 73 files**:

| object | files |
|---|---|
| LIBGPU SYS | 17 (`sys`, `ClearOTag`, `ClearOTagR`, `PutDispEnv`, `GetDispEnv`, `drawarea`, `gpu_build`, `vram_transfer`, `dma_queue`, `LoadImage2`, `GPU_memset`, and 6 game-named `Gpu_*` pieces: `Gpu_BuildDrawModeCmd`, `Gpu_SendOtChain`, `Gpu_DmaVramTransfer`, `Gpu_SubmitPacket`, `Gpu_ResetDmaWaitTimer`, `Gpu_RestoreDmaCallback`) |
| LIBCD BIOS | 8 |
| LIBCD SYS | 6 |
| LIBAPI PAD | 6 |
| LIBDS DSSYS_2 | 5 |
| LIBDS DSREADY | 5 |
| LIBSPU SPU | 4 |
| LIBCARD PATCH | 4 |
| LIBPAD PADPORTD | 4 |
| LIBETC INTR | 3 |
| LIBCD C_011 | 3 |
| LIBMATH MULDF3 | 2 |
| LIBAPI PATCH | 2 |
| LIBDS DSFILE | 2 |
| LIBSPU S_R/S_W | 2 (verify) |

The evidence report independently marks most of these "strong" joins (G0981,
G1041, G1074, G1040, G0971, G1051, G1060, G1132, G1113, G1072, G1112, G1048).

The plausible PsyQ layout is what Sony shipped: one source per `.OBJ` in each
`LIB*.LIB` archive.

- LIBAPI and LIBC are mostly BIOS A/B/C-table veneers (`a50.c`, `c21.c`…) plus
  a few C objects (`pad`, `patch`, `sprintf`).
- LIBGPU is one big `sys.c` plus primitive objects `p00…p36`.
- LIBGTE is mostly assembler objects (`MSC00`, `RATAN`, …), which stay GNU-as
  units under SDK names.
- LIBCD has `bios`, `sys`, `c_0NN`, `iso9660`, the `St*` stream objects.
- LIBSPU has the many tiny `s_*` objects.
- LIBDS, LIBPAD, LIBCARD, LIBETC (`intr`, `vsync`) and LIBMATH follow the same
  pattern.

Target: **~210 files, one per object**. The 6 game-named LIBGPU pieces take
the SDK labels when they fold into `libgpu/sys.c`, after a marker probe like
the one below.

### 1.3 Overlays (`src/overlays`, 190 overlays + `room_lib`)

| | value |
|---|---|
| overlays (manifests) | 190; **22** have own C, **167** link only shared `room_lib` units, 1 has no C |
| own C units / files | **432** (+ 53 shared files in `room_lib/`, 684 manifest references) |
| single-function own files | **325** (75%); 34 are 5-line `.inc` template instantiations |
| `func_XXXXXXXX.c` file names | **149** (ratchet baseline overlays.address_files = 149) in 17 overlays: scene_e08 49, room_m273 20, sys_reset 14, room_m256 10, room_m350 10, menu_memcard 8, room_m318 8, scene_e19 7, fx_common 5, scene_e22 5, boot_display 4, scene_e18 3, room_m086 2, m089/m123/m272/m349 1 each |
| `Role_ADDRESS.c` instance names | 76 (e.g. `RoomEffect_JointBeacon_8018FDC4`, `RoomM272_Nop_8018F188`, `Memcard_SetVideoDisplay_80121004`) |
| `func_` functions defined in overlay C | ~580 distinct; `D_` names referenced ~911 |
| marker signatures | 394 of 432 own units are default; `--expand-div` 19; others are singletons (`-G8` 3, `-fno-cse-skip-blocks` 3, `-fno-schedule-insns` 2, GNU-as 6, gcc-2.8.1 1, `-ffixed-*` 1, …) |
| `--expand-div` neutrality | **all 394 default own units compile byte-identically with `--expand-div`** (probe) |
| room_lib suffix family | `RoomFx_MotionParticles{,_2,_3}` (split by `-fno-strength-reduce`) |
| overlay dir names | 153 rooms have place suffixes; `room_m350` has none; 8 carry `+1/+2` extraction tails (`room_m126+1_hospital`…); 22 `scene_eNN` unnamed; `sys_reset` (MDEC/LIBPRESS) and `render_clip` (data-only) flagged as misleading in CONVENTIONS.md |

Contiguous runs of own units in manifest order break at shared units,
non-`c` entries and markers. With `{}` and `--expand-div` treated as one set,
**432 own files collapse into 58 runs**. Only 2 runs have foreign rodata
between members: `boot_display` (`boot_display_font_tables_pad`) and
`room_m256` (`RoomEffect_SpriteSeeds_8018F208`). Only 2 runs instantiate the
same `.inc` template twice, which is a static-name collision risk. After
those merges, **7 `func_` files remain alone**, each because of its own flag:

- `fx_common/func_80195BC8` (-fno-schedule-insns)
- `menu_memcard/func_80127848` (-fno-cse-skip-blocks)
- `room_m123/func_8019537C` (sits between shared units)
- `room_m273/func_8019A398` (-fno-schedule-insns)
- `scene_e18/func_80192E60` (-fno-cse-skip-blocks)
- `sys_reset/func_8010BE70`
- `sys_reset/func_8010BEFC` (GNU-as)

---

## 2. Target layout for `src/main` (game code)

Principles:

- Modules are **contiguous address ranges** in `configs/USA/main.yaml`.
- Boundaries go at binary split evidence first, then at responsibility
  changes where the evidence is neutral.
- Directories are the subsystem taxonomy already in `docs/CONVENTIONS.md`,
  using its "Target" column: `task`→`script`, `engine`→`fieldfx`, `gpu`→`draw`,
  `table`→`text`.
- `main/`, `event/`, `sys/` and `overlay/` dissolve.
- File names are lower snake, with no prefix redundancy: `battle/battle_hud.c`,
  not `Battle_DrawHPBar.c`.

**339 files → 65 modules** (69 files, because 4 GNU-as units must stay
separate under the source policy), in 22 directories:

| dir | modules (old units) |
|---|---|
| akao | spu(1), voice(1), voice_bank(1), sequencer(1) — each sits between PsyQ objects |
| anim | anim(2) |
| asset | asset(5) |
| aya | aya_stats(4) |
| battle | battle_actor(6), battle_setup(14), battle_attack(7), battle_turn(10), battle_main(9), battle_context(6), battle_hud(10), battle_cmd(13) |
| boot | main(1), boot(5) |
| cdrom | cdrom(4) |
| draw | textbox(3), font(3), draw(4), prim_queue(6) |
| entity | entity(4) |
| field | actor_collision(6) |
| fieldfx | object, sprite, collision(3), model, glow(4), burst(6), anim, shapes(3), effects_sprite(8), effects_emitter(10), fx_math(3) |
| item | inventory_list(16), item_use(6), inventory_swap(9) |
| math | fixed(3: 1 C + 2 GNU-as) |
| memcard | memcard(2) |
| menu | save_slot_draw(2), menu_status(11), menu_equip(8), menu_skill(7), menu_item(5), menu_main(4), menu_save(8), menu_inventory(10), menu_list(10), menu_queue(1), menu_widget(1) |
| pm | process(3) |
| render | object(6), render(6) |
| save | save_data(10) |
| scene | scene_load(5), player_move(2) |
| script | script_pool(2), script_expr(2), script_actor_ops(8), script_flow_ops(10), script_system_ops(11), sfx_queue(1) |
| text | text_table(1) |
| time | game_time(2) |

Optional second step, needing a decision: fold the one-module directories
(anim→render, time+pm→boot, text→menu, aya→battle, field+entity→entity) to
reach ~14 directories. CONVENTIONS.md says "Do not create a directory for one
file". The plan creates none; it only keeps a few existing ones.

### 2.1 Status of the 65 modules

| status | modules | meaning |
|---|---|---|
| READY | 14 (67 units) | one marker set compiles every member byte-identically, no foreign rodata, no split evidence: `script_flow_ops`(10), `save_slot_draw`(2), `game_time`(2), `anim`(2), `memcard`(2), `menu_inventory`(10), `process`(3), `player_move`(2), `fieldfx/collision`(3), `glow`(4), `burst`(6), `shapes`(3), `effects_sprite`(8), `effects_emitter`(10) |
| READY-AFTER-DATA | 3 | markers fine, claim one rodata blob first: `script_actor_ops` (rodata_0009AC), `battle_context` (rodata_001324), `battle_hud` (rodata_0015FC) |
| PARTIAL | 29 | merge inside compatible marker parts now, unify the rest later; e.g. `item/inventory_list` 16→2 parts, `battle_setup` 14→5, `battle_cmd` 13→5, `script_system_ops` 11→5, `menu_status` 11→3 |
| BLOCKED-MARKERS | 5 | no two neighbours share a set yet: `script_pool`, `script_expr`, `draw/font`, `menu_main`, `draw/draw` |
| BLOCKED-POLICY | 1 | `math/fixed` (2 of 3 are GAME_ASM units; `check_source_policy.py` requires those to be alone) |
| SINGLE | 13 | rename only (akao ×4, fieldfx object/sprite/model/anim, boot/main, text_table, menu_queue, menu_widget, script/sfx_queue) |

How the file count drops:

| stage | files |
|---|---|
| after renames only | 339 |
| after READY merges | 286 |
| after every currently marker-compatible merge (rodata-blocked groups left alone) | 184 |
| after data claims and marker unification (end state) | 69 |

Modules that cannot be probed here, because they are -g3 or line-sensitive:
`battle_actor` (`Menu_MainUpdate`, `Entity_UpdateTimers`), `battle_attack`
(`Battle_PhaseAndEnemyEffectFlow`), `boot` (`Gpu_InitPipeline`), `menu_save`
(`Menu_OpenRenameScreen`), `inventory_list` (`Sfx_DrawSlotRow`), `item_use`
(`Inv_ItemActions`), `save_data` (`Save_MetadataWindow`). -g3 changes old GCC's
scheduling through line notes, so a -g3 unit merges only with -g3 neighbours.
Treat these as hard boundaries until a real merge build says otherwise.

### 2.2 Mapping old file → new module

The full table is in the Appendix, generated by `scratchpad/reorg/mkmap.py`;
the CSV is `scratchpad/reorg/mapping.csv`. Each entry gives the module, the
evidence, the blockers, and for every old file its markers and its "part"
number. Files with the same part number can be merged now.

---

## 3. Overlays

1. **Keep `room_lib/` one unit per method.** This is documented policy: rooms
   replace single class methods. Merge only the flag-split
   `RoomFx_MotionParticles{,_2,_3}` once they compile under one set; probe
   first, as with `FieldEng_CollisionQueries`.
   Status (2026-10-13, Step 10): the class methods stay one unit each; a
   module and its own class, which always follow each other, became one
   unit, and the motion particle set is still split.
2. **Directory names.** Make one config-wide rename commit per item, covering
   yaml, sym file, src dir and CI cache key:
   - `room_m350` gets its text-bank suffix.
   - Drop the 8 `+1/+2` tails. If a tail collides with an existing directory,
     keep a disambiguating suffix.
   - `sys_reset` → `mdec_diag`; its `DecDCTReset`/`DecDCTvlc{,2}` are PsyQ
     LIBPRESS, so put them under `psyq_libpress/`-style object names inside
     the overlay.
   - `render_clip` gets a data-oriented name.
   - Name the 22 `scene_eNN` as scenes get identified.
3. **One file per run** (58 TUs from 432 files). Naming scheme:
   - The main run of a room or scene is `<overlay>/<short>.c`
     (`room_m273_liberty/m273.c`, `scene_e08/e08.c`).
   - The 4-function class preludes are `<short>_class.c`. These are the
     `return 0` / init / message-switch / `return 0` runs at the start of
     m256, m273 and e19, matching the `include/pe1/room_module.h` slot
     layout.
   - Further runs get role names taken from their dominant functions:
     `fx_common/{camera,effects,buffers,render_op}.c`,
     `menu_memcard/{video,glint,image,list_screen}.c`,
     `boot_display/display.c`.
   - A `Role_ADDRESS` instance file (`#define NAME func_X` +
     `#include "Role.inc"`) becomes one `#define`/`#include`/`#undef` block
     inside the run's file.

   Per-overlay result:

   | overlay | own files → TUs | notes |
   |---|---|---|
   | scene_e08 | 73 → 2 | `SceneE08_ModuleStart` sits between shared units |
   | fx_common | 53 → 10 | flagged singletons |
   | menu_memcard | 49 → 6 | |
   | room_m350 | 43 → 3 | `-G8` singleton |
   | room_m273 | 39 → 4 | |
   | boot_display | 26 → 1 | split at the font-table pad, or claim it |
   | scene_e19 | 23 → 2 | |
   | room_m318 | 21 → 2 | |
   | sys_reset | 20 → 9 | 4 GNU-as + gcc-2.8.1 units must stay separate |
   | room_m256 | 16 → 2 | |
   | scene_e22 | 16 → 1 | |
   | scene_e18 | 9 → 3 | |
   | 10 small rooms and scenes | 1–3 each | |

4. **The 7 leftover `func_` files** are renamed by role. The agent reads the
   code, its caller and the slot it occupies in `g_RoomDrawList`,
   `g_RoomUpdateList` or `g_RoomInitList`, or in the class table, then names
   it `<Prefix>_<Effect><Draw|Update|Init|Callback>`. This takes
   `overlays.address_files` from 149 to 0.
5. **Symbol renames** of the ~580 `func_` and ~911 `D_` names inside overlays
   are a separate, ongoing pass with `tools/scripts/apply_names.py` and the
   `sym.<overlay>.txt` files. Merging does not rename symbols. File merges
   must not be reported as naming progress beyond the file-name ratchet.

---

## 4. Order of work

Each step is its own small commit. Every step runs this gate:

1. `make check`
2. `make overlay-check-all` (or `overlay-check` per touched overlay, then
   `-all` before push)
3. `make ci`: check-sources, source-policy, debt-check, organization-check
4. `make organization-baseline` and `make debt-baseline` whenever counts drop

Both ratchets fail on *improvement* with a stale baseline, so the lowered
baseline must ship in the same commit. Push with `make progress` per the
standing workflow.

### Step 0: tooling (2 rounds)

- `tools/scripts/reorg_move.py MAP.csv [--dry-run]`:
  - `git mv` each file and its sibling `*_externs*.h` / `.inc` (only 4
    externs headers and 2 `.inc` in main game code; 293 externs headers
    tree-wide).
  - Rewrites the `c`/`.rodata`/`.data`/`.sdata` entries in
    `configs/USA/main.yaml` and the overlay yamls, plus the yaml comments that
    name units.
  - Rewrites `configs/USA/original_asm_evidence.json` `source` fields (10
    entries).
  - Rewrites path references in `tools/tests/*.py` (~100 legacy tests; not a
    CI gate), `docs/*.md` (~50 path-qualified references) and the
    `include/pe1/*.h` comments (34 headers).
  - Normalises `#include "../../../tools/m2c/m2c_macros.h"` to
    `"m2c_macros.h"`. cc.sh already passes `-I tools/m2c`, and the relative
    form is depth-dependent (7 files).
  - Prints a summary.
- `tools/scripts/reorg_merge.py --module NEW --units A B C --markers SET`:
  - Concatenates the sources in manifest order and hoists or de-duplicates the
    `#include` lines.
  - Keeps each unit's externs header for now.
  - Writes one marker header.
  - Collapses the yaml entries: one `c` entry at the first offset, one
    `.rodata` entry at the first owned rodata offset.
  - Refuses if any foreign `rodata`/`data` entry lies between members' entries.
  - Refuses if one `.inc` is included twice without `#undef` guards.
  - Refuses if a GAME_ASM unit is involved.
- `tools/scripts/marker_probe.py`: promote `scratchpad/reorg/probe.py`. It
  compiles scratch copies under candidate marker sets into `build/probe/` and
  compares every loadable section, relocation and symbol. Run it before every
  merge.
- Optional: teach `main_tu_evidence.py` its `--out` outside the repo. It
  currently crashes on `relative_to` after writing.

### Step 1: pure renames, no merges (3 rounds main, 1 round overlays)

Byte-safe: no source uses `__FILE__`/`__LINE__`, and path depth stays
`src/main/<dir>/`.

| batch | change | moves |
|---|---|---|
| 1a | directory renames from CONVENTIONS: `task/`→`script/`, `engine/`→`fieldfx/`, `gpu/`→`draw/`, `table/`→`text/` | ~84 |
| 1b | dissolve `main/` (14), `event/`, `sys/`, `overlay/` into their modules' directories | |
| 1c | remaining cross-directory moves from the mapping (`battle/Inv_*`→`item`, `menu/Pad_*`, `boot/Gpu_*`, `field/Entity_*`…) | |
| 1d | overlay directory renames (§3.2) | one per overlay |

Main total: 163 cross-directory moves in batches 1a–1c.

### Step 2: READY merges (2 rounds)

Merge the 14 modules, 67 files → 14:

- Start with the field effect engine (5 modules, 31 files, all default
  markers).
- Then `script_flow_ops`, `menu_inventory`, `process`, `memcard`, `anim`,
  `game_time`, `player_move` and `save_slot_draw`.
- Delete the stale `-fno-schedule-insns` comment for CollisionQueries in
  `main.yaml`.

### Step 3: overlay merges (3 rounds)

- 432 → 58 files, overlay by overlay. scene_e08 (72→1) and fx_common go
  first.
- Run `overlay-check` for each overlay plus every overlay that links a touched
  `room_lib` unit.
- Then name the 7 leftover `func_` files (1 round).

### Step 4: PARTIAL merges within compatible parts (3–4 rounds)

29 modules, 339→~184 files tree-wide in main.

### Step 5: data claims (1–2 rounds)

- Define `rodata_0009AC`, `rodata_001324` and `rodata_0015FC` as `const`
  objects in their modules, then merge `script_actor_ops`, `battle_context`
  and `battle_hud`.
- Same for the 2 overlay runs.

### Step 6: marker unification (6–12 rounds, open-ended)

There are 74 incompatible boundaries; 43 involve a cc1 -G difference. Use the
documented C-shape recipes:

- incomplete-array declarations, so a -G8 unit keeps absolute addressing (the
  `RoomFloorY` pattern);
- the G0341/G0835 loop-shape recipes;
- shared declarations through `include/pe1`.

Re-run `marker_probe.py` after each change. Accept leftovers, especially
`-g3` units, as role-named sub-files of the module (`battle_attack_flow.c`),
not numbered parts.

### Step 7: PsyQ object merges (2–3 rounds)

Merge 73 files into 15 objects, with LIBGPU SYS (17 files) first. Probe
markers first; most differ only by the `_N` split reason recorded in each
file's header.

Status (2026-10-12): the marker probe merged every run of parts that share
a lev-0 set (268 to 250 files) and the split units now carry their
object's name (`libgpu/sys_N.c`, `libcd/bios_N.c`, ...). The 17 objects
still split, and why, are listed in CODE_ORGANIZATION.md ("PsyQ objects
split over several units"): neighbouring parts need different compilers,
so one-file-per-object is out of reach with the stock toolchain.

### Step 8: symbol renames (continuous)

Covers 271 `func_` in main (mostly `fieldfx`) and ~580 in overlays. This is
independent of file layout, and smaller files no longer help it.

Estimate: about **20–30 agent rounds** in total. Steps 0–3 take ~10 rounds and
already deliver 339→286 main files and 432→58 overlay files. Step 6 is the
long tail.


### Step 9: snake-case file names (2026-10-13)

Decision: the `src/main` game units (outside `psyq/`) are named like the
original developers would name C files, lower snake case without the
directory or function prefix (`battle/encounter.c`, `akao/voice_bank.c`,
`fieldfx/geometry.c`), instead of the function-style names the merges left
(`Battle_EncounterFlow.c`). Function names did not change. All 99 units and
the two private `.inc` templates were renamed with `reorg_move.py`
(`tools/reorg/maps/step9_snake_case_names.csv`); the module names of §2 and
the Appendix (`battle/battle_hud.c`, ...) are superseded by these, and the
`current` column of `docs/reorganization_mapping.csv` gives the unit that
now holds each original file. Single-function strays left in a foreign
directory moved to their subsystem: `battle/Menu_MainUpdate.c` to
`menu/main_update.c`, `item/Menu_OpenItemUsePanelAtIndex.c` to
`menu/item_use_panel.c`, `boot/Draw_InitPipeline.c` to `draw/pipeline.c`,
the `draw/Render_*` units to `render/` and `draw/Math_SqrtApprox3.c` to
`math/sqrt_approx.c`. `battle/Entity_HitAndDropTable.c` and
`battle/Entity_UpdateTimers.c` compile as one `-g3` unit and became
`entity/combat.c`; the other strays have no compatible neighbour of their
subsystem and stay separate units. `make organization-check` now counts
names that break the convention (`unconventional_files`).

The shared overlay code followed on 2026-10-13
(`tools/reorg/maps/step9_overlay_shared_units.csv`, rows also in
`docs/reorganization_mapping.csv`): `mdec_diag` mirrors its PsyQ LIBPRESS
objects (`decdct.c`, `mdec_wait.c`, `mdec_status.c`, `vlc_build.c`,
`vlc.c`, `vlc_c.c`; the seven units from `DecDCTReset` to
`MdecDiag_ReceiveOutput` are one GNU-as unit), and `fx_common` is
`camera.c` (the camera nodes and the `-G8` render ops), `effects.c`,
`motion_endpoints.c`, `camera_path.c`, `effect_init.c` and
`model_buffers.c`. `fx_field` has no C of its own. What stays apart and
why is in `docs/CODE_ORGANIZATION.md`.

The shared-code overlays followed: `boot_display` went from 21 units to 6
and `menu_memcard` from 28 to 8, named by role in lower snake case
(`tools/reorg/maps/step9_overlay_shared_units.csv`). What kept a "segment
is at most one unit" limit there was the link base of copied code; units
without local jumps reveal no base and now share the segment of their
neighbours (for `boot_display` the retail callback tables and calls show
that the whole overlay runs at 0x80122FA4). The pairs that stay apart,
and why, are in CODE_ORGANIZATION.md ("Overlay translation units").

### Step 10: room library units (2026-10-13)

The shared units in `src/overlays/room_lib/` follow the same naming
(`RoomEffect_OrbitFlare.c` is `orbit_flare.c`, `RoomLib_Spawn6.c` is
`class_spawn.c`, `RoomLib_SetArgs3.c` is `script_args.c`), mapped in
`tools/reorg/maps/step10_room_lib_units.csv` and in the room_lib rows of
`docs/reorganization_mapping.csv`. Five pairs that every linking overlay
places back to back became one unit (`drop_field.c`,
`ground_eruption.c`, `orbit_set.c`, `three_particles.c`,
`class_register_active.c`), single-instance templates were written out
in their unit, and the per-unit `_externs.h` headers were folded into
existing headers: 90 files to 54. `make organization-check` counts
`room_lib` names that break the convention in the overlays scope
(ratcheted at zero). What stays apart and why is listed in
CODE_ORGANIZATION.md, "Room library units".

---

## 5. Risks and mitigations

1. **Merged-TU codegen is not the sum of per-unit codegen.**
   - GCC 2.7.2 de-duplicates identical string literals inside one TU.
   - It may pool constants differently.
   - A symbol defined in the same TU changes `-G` small-data addressing. This
     affects few cases here, because data is mostly asm blobs.
   - `static` and template helper names can collide.
   - The probe proves per-unit equivalence only. Every merge still needs the
     full `make check` / `overlay-check`.
2. **Rebases lose edits in merged files.** This is a known incident (memory:
   header-conflicts state). Merge only on a fresh `main`, one directory per
   commit, and never with parallel agents in the same directory. Check that
   goto and pin totals are unchanged against `crutch_debt` before pushing.
3. **Ratchet gates fail on improvement.** Commit the regenerated
   `tools/organization_debt_baseline.json` and `tools/crutch_debt_baseline.json`
   with each merge.
4. **Policy text.** `docs/CODE_ORGANIZATION.md` says "Function count and
   aesthetics alone are not reasons to invent source boundaries". The
   responsibility boundaries placed where evidence is neutral need an explicit
   policy amendment from the user. Otherwise the conservative alternative is:
   - merge only the evidence-backed groups (23 groups, 111 files) plus the
     READY field-effect families;
   - leave neutral boundaries as they are.
5. **Tooling keyed by path:** objdiff config, `docs/match_inventory.json`,
   progress badges and the source browser.
   - Regenerate them with `make objdiff-config progress`.
   - Progress history per unit restarts; totals do not change.
6. **GNU-as and gcc-2.8.1 units** can never join a C module under the current
   source policy: 4 in main, 5 in `sys_reset`, 1 in `scene_e20`
   (`-ffixed-*`). Keep them as role-named siblings.
7. **Probe blind spots:** -g3 and line-sensitive units (8), and marker sets
   not tried. The 184/69 file counts are bounds, not promises.

---

## Appendix: src/main mapping (old file → new module, in text order)

Part = files with the same number in a module compile byte-identically under one marker set today (probe), so they can merge now. Markers: cc1 = CC1_FLAGS, as = MASPSX_FLAGS (comm = --use-comm-section, div = --expand-div, noli = --dont-expand-li), FG0 = MASPSX_FORCE_G0, aspsx = ASPSX_VERSION.

#### `boot/main.c` — SINGLE (1 units, 1 functions, .text +0x002A0C; entry point; isolated by script code)

| old file | markers | part |
|---|---|---|
| `boot/Boot_MainLoop.c` | default | 1 |

#### `script/script_pool.c` — BLOCKED-MARKERS (2 units, 6 functions, .text +0x002CF8; node pool + free list (G0001 private join))
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `task/Task_NodePoolSetupFlow.c` | cc1 -G8 -fno-strength-reduce, as -G8 | 1 |
| `task/Task_NodePoolFlow.c` | cc1 -G8, as noli comm -G8, FG0, aspsx 2.21 | 2 |

#### `script/script_expr.c` — BLOCKED-MARKERS (2 units, 12 functions, .text +0x003050; expression/Vec3 opcodes, node actions (private join))
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `task/Task_ValueOps.c` | as div | 1 |
| `task/Task_NodeActions.c` | cc1 -G8, as div comm -G8, FG0 | 2 |

#### `script/script_actor_ops.c` — READY-AFTER-DATA (8 units, 20 functions, .text +0x004A28; actor/camera/model/item opcodes)
Blockers: unowned rodata blob between members: rodata_0009AC

| old file | markers | part |
|---|---|---|
| `entity/Entity_QueryAndDistance.c` | default | 1 |
| `task/Task_MemCardPositionCameraOps.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_PolyAndSceneLoadOps.c` | default | 1 |
| `task/Task_PlayEntityAnim.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_CopyEntityRenderObject.c` | default | 1 |
| `task/Task_ActorModelOps.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `main/MapSelectionFlow.c` | default | 1 |
| `task/Task_ItemAndSoundOps.c` | cc1 -G8, as comm -G8, FG0 | 1 |

#### `script/script_flow_ops.c` — READY (10 units, 40 functions, .text +0x007110; dispatcher, queue, node/flow/bit opcodes)

| old file | markers | part |
|---|---|---|
| `task/Task_DispatchScriptCmd.c` | cc1 -G8, as comm -G8, FG0, aspsx 2.70 | 1 |
| `task/Task_ShadowFlagOps.c` | default | 1 |
| `task/Task_BeginSceneTransition.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_GetPlayerInputFlag.c` | default | 1 |
| `task/Task_RunQueue.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_NodeAndFlowOpcodes.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `entity/Entity_ComputeVelocityToTarget.c` | default | 1 |
| `task/Task_ActorAndNodeFieldOps.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_BitAndActionOpcodes.c` | default | 1 |
| `task/Task_WaitAnimFrameMatch.c` | cc1 -G8, as comm -G8, FG0 | 1 |

#### `script/script_system_ops.c` — PARTIAL (11 units, 149 functions, .text +0x0083B4; state/flag, system, field-entity, scene opcodes)
Blockers: markers: 5 compatible parts

| old file | markers | part |
|---|---|---|
| `task/Task_StateAndFlagOpcodes.c` | as comm -G8 | 1 |
| `entity/Entity_TaskAnimationOps.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `task/Task_SystemCommandOps.c` | default | 1 |
| `task/Task_DrawYield.c` | cc1 -G8, as -G8 | 2 |
| `entity/Entity_AttachmentAndControl.c` | default | 3 |
| `task/Task_LoadWaitAndItemOps.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `task/Task_FieldEntityCommands.c` | default | 3 |
| `task/Task_WaitEntityFlags8.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `task/Task_RotateEntityAngle.c` | default | 3 |
| `task/Task_SetNodeFlag80.c` | cc1 -G8, as -G8 | 4 |
| `task/Task_SceneCommandOps.c` | default | 5 |

#### `field/actor_collision.c` — PARTIAL (6 units, 19 functions, .text +0x00ACAC; G0227 medium join (collision db + floor resolution))
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `entity/Entity_AnimationState.c` | as div | 1 |
| `entity/Entity_CollisionDatabase.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `entity/Entity_UpdateList.c` | default | 1 |
| `field/Geo_ActorFloorResolution.c` | cc1 -G8, as div comm -G4, FG0, aspsx 2.70 | 1 |
| `field/Entity_ApplyCollisionResponse.c` | cc1 -G8, as div -G8 | 2 |
| `entity/Entity_CheckActionIdMatch.c` | default | 2 |

#### `battle/battle_actor.c` — PARTIAL (6 units, 10 functions, .text +0x00DB40; G0232 strong join (literal/order/private))
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_MainUpdate.c` | cc1 -G1 -g3, as div comm -G1, FG0 | 1 |
| `entity/Entity_HitAndDropTable.c` | cc1 -G8, as div comm -G8, FG0 | 2 |
| `entity/Entity_UpdateTimers.c` | cc1 -G1 -g3, as div -G1 | 3 |
| `battle/Battle_SubActionStep.c` | default | 4 |
| `main/Battle_ApplyEnemyAttack.c` | default | 4 |
| `battle/Battle_SetupEnemyAnims.c` | default | 4 |

#### `battle/battle_setup.c` — PARTIAL (14 units, 18 functions, .text +0x011474; battle init, target sort, pad sync, script entry)
Blockers: markers: 5 compatible parts

| old file | markers | part |
|---|---|---|
| `battle/Battle_ActionLifecycle.c` | cc1 -G2, as -G2 | 1 |
| `battle/Battle_HaltOnPositiveX.c` | cc1 -G1, as -G1 | 1 |
| `battle/Battle_InitEntityColors.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `save/Save_ResetGlobalFlags.c` | as comm -G8 | 2 |
| `main/Battle_Init.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `menu/Pad_GetMenuPressedBitOrDisabled.c` | as comm -G8 | 2 |
| `menu/Menu_BattleActiveQuery.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `battle/Battle_DispatchEntityEffect.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `main/Battle_CheckEvasion.c` | as div | 3 |
| `main/Battle_BuildTargetList.c` | cc1 -G8, as comm -G8, FG0 | 4 |
| `battle/Battle_TargetSort.c` | default | 4 |
| `battle/Battle_PadStateSyncFlow.c` | cc1 -G8, as comm -G8, FG0 | 4 |
| `battle/Battle_FlushScriptSounds.c` | cc1 -G8, as comm -G8, FG0 | 4 |
| `battle/Battle_StepScriptEntry.c` | cc1 -G2, as -G2 | 5 |

#### `battle/battle_attack.c` — PARTIAL (7 units, 11 functions, .text +0x012738; G0250 strong join (shared literals))
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `main/Battle_PlayerTurnFlow.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `main/Battle_PhaseAndEnemyEffectFlow.c` | cc1 -G8 -g3, as div comm -G8, FG0 | 2 |
| `main/Battle_FinalizeAttackResult.c` | cc1 -G8 -fno-force-mem, as comm -G8, FG0 | 3 |
| `battle/Battle_ResolveHitOnTimer.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `main/Battle_ApplyDamage.c` | default | 3 |
| `main/Battle_ApplySpellEffect.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `battle/Battle_StepAyaAction.c` | cc1 -G8, as div comm -G8, FG0, aspsx 2.77 | 3 |

#### `battle/battle_turn.c` — PARTIAL (10 units, 25 functions, .text +0x015CBC; escape/target/enemy-turn/movement)
Blockers: markers: 3 compatible parts · unowned rodata blob between members: rodata_001080

| old file | markers | part |
|---|---|---|
| `battle/Battle_DispatchSpecialAction.c` | default | 1 |
| `battle/Battle_RollEscapeChance.c` | cc1 -G0, as comm -G0 | 1 |
| `battle/Battle_TargetAndEnemyTurnFlow.c` | cc1 -G0, as div comm -G8, aspsx 2.70 | 1 |
| `battle/Battle_AdvanceTurnSlot.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `battle/Battle_TargetFade.c` | cc1 -G0, as comm -G8 | 1 |
| `battle/Battle_MarkActiveEntities.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `battle/Battle_ActionSlotAndEnemy.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `battle/Battle_StepEnemyMovement.c` | cc1 -G0, as div -G0 | 3 |
| `main/Battle_UpdateEntityFacing.c` | default | 3 |
| `battle/Battle_StepEntityAnimState.c` | default | 3 |

#### `battle/battle_main.c` — PARTIAL (9 units, 19 functions, .text +0x019B88; G0263 medium join (encounter..post-battle))
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `save/Save_InitSystem.c` | cc1 -G8, as -G8 | 1 |
| `battle/Battle_EncounterInitializationFlow.c` | cc1 -G0, as comm -G8, FG0 | 2 |
| `battle/Battle_Update.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `battle/Battle_StepVictory.c` | cc1 -G0, as comm -G8, FG0 | 3 |
| `battle/Battle_StepLevelUp.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `battle/Battle_StepPostBattle.c` | cc1 -G0, as div comm -G8, FG0 | 3 |
| `battle/Battle_EscapeAndPhaseInitFlow.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `battle/Battle_AyaEquipStateFlow.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `battle/Battle_EnemySlotLifecycle.c` | default | 3 |

#### `battle/battle_context.c` — READY-AFTER-DATA (6 units, 12 functions, .text +0x020210; context accessors (no evidence either way))
Blockers: unowned rodata blob between members: rodata_001324

| old file | markers | part |
|---|---|---|
| `entity/Entity_AnimationEvents.c` | default | 1 |
| `battle/Battle_ContextAccess.c` | default | 1 |
| `battle/Battle_GetEnemyContextField.c` | default | 1 |
| `entity/Entity_WriteFieldByCmd.c` | default | 1 |
| `battle/Battle_AngleAndDistanceFlow.c` | default | 1 |
| `battle/Battle_RandomChecks.c` | default | 1 |

#### `battle/battle_hud.c` — READY-AFTER-DATA (10 units, 21 functions, .text +0x021094; G0283 strong join (HP bar/status overlay))
Blockers: unowned rodata blob between members: rodata_0015FC

| old file | markers | part |
|---|---|---|
| `battle/Battle_DrawHPBar.c` | cc1 -G8, as -G8 | 1 |
| `main/Battle_DrawStatusOverlay.c` | default | 1 |
| `main/Battle_StatusPrimitiveFlow.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `main/Battle_StatusNumberFlow.c` | default | 1 |
| `battle/Battle_DrawStatusPanel.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `gpu/Gpu_SetupSprites.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `menu/Menu_ActiveWindowMode.c` | as comm -G8 | 1 |
| `task/Task_SignalExit.c` | default | 1 |
| `battle/Battle_ActiveAndEnemyStatusFlow.c` | cc1 -G8, as div comm -G8, FG0, aspsx 2.70 | 1 |
| `render/Render_AnimationFrame.c` | cc1 -G8, as comm -G8, FG0 | 1 |

#### `menu/save_slot_draw.c` — READY (2 units, 2 functions, .text +0x02515C; save slot metadata draw (private join))

| old file | markers | part |
|---|---|---|
| `save/Save_DrawSlotMetadata.c` | cc1 -G8, as -G4 | 1 |
| `menu/Menu_SaveOverlayDraw.c` | cc1 -G8, as -G8 | 1 |

#### `entity/entity.c` — PARTIAL (4 units, 15 functions, .text +0x025710; entity pool, frame update, allocation)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `entity/Entity_PoolInit.c` | cc1 -fno-strength-reduce, as comm -G8 | 1 |
| `scene/Scene_LoadMap.c` | cc1 -G8, as comm -G8 | 2 |
| `entity/Entity_FrameUpdate.c` | cc1 -G8, as comm -G4, FG0 | 3 |
| `entity/Entity_AllocationAndContacts.c` | default | 3 |

#### `time/game_time.c` — READY (2 units, 5 functions, .text +0x0275F8; play-time counters (private join))

| old file | markers | part |
|---|---|---|
| `time/GameTime_Counters.c` | default | 1 |
| `scene/Scene_TickTimers.c` | default | 1 |

#### `math/fixed.c` — BLOCKED-POLICY (3 units, 1 functions, .text +0x02788C; fixed-point helpers (2 of 3 are GNU-as units))
Blockers: 2 GAME_ASM (GNU-as) unit(s) must stay separate files (source policy) · probe could not test (-g3/line-sensitive): Math_FixedMul, math_fixed

| old file | markers | part |
|---|---|---|
| `math/Math_FixedMul.c` | GNU-as | 1 |
| `math/Math_FixedDivide.c` | default | 2 |
| `math/math_fixed.c` | GNU-as | 3 |

#### `draw/textbox.c` — PARTIAL (3 units, 16 functions, .text +0x0278DC; draw-mode packets + textbox)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `gpu/Gpu_DrawModePackets.c` | default | 1 |
| `menu/Menu_SetEquipSlotIndex.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_TextboxDraw.c` | cc1 -G8, as comm -G8, FG0, aspsx 2.70 | 2 |

#### `draw/font.c` — BLOCKED-MARKERS (3 units, 18 functions, .text +0x029154; font glyph load/selection)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `sys/Sys_ReportError.c` | default | 1 |
| `render/Render_FontGlyphLoad.c` | cc1 -fno-schedule-insns | 2 |
| `render/Render_FontGlyphSelection.c` | as div | 3 |

#### `anim/anim.c` — READY (2 units, 4 functions, .text +0x02A19C; bone rotations)

| old file | markers | part |
|---|---|---|
| `scene/Scene_CheckFlagBits.c` | default | 1 |
| `anim/Anim_BoneRotations.c` | default | 1 |

#### `render/object.c` — PARTIAL (6 units, 34 functions, .text +0x02A888; G0320 medium join (object draw))
Blockers: markers: 5 compatible parts

| old file | markers | part |
|---|---|---|
| `render/Render_VertexTransforms.c` | cc1 -G8, as -G8 | 1 |
| `render/Render_EntityAndTexturedQuadFlow.c` | default | 2 |
| `render/Render_DrawObjectAndClut.c` | cc1 -G8, as -G8 | 3 |
| `render/Render_DrawObjectAlt.c` | default | 4 |
| `render/Render_DrawObjectVariant.c` | cc1 -G8, as div | 4 |
| `render/Render_ObjectSetupAndAnim.c` | default | 5 |

#### `boot/boot.c` — PARTIAL (5 units, 13 functions, .text +0x02EE10; G0344 medium join (init + frame loop))
Blockers: markers: 2 compatible parts · binary split evidence: Boot_RenderFlagsAndPadInput|Gpu_InitPipeline:gprofile

| old file | markers | part |
|---|---|---|
| `boot/Boot_InitSystemFlow.c` | default | 1 |
| `boot/Boot_RenderFlagsAndPadInput.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `boot/Gpu_InitPipeline.c` | cc1 -G0, as comm -G8, FG0 | 1 |
| `boot/Boot_FrameLoop.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `gpu/Gte_LightingState.c` | default | 2 |

#### `memcard/memcard.c` — READY (2 units, 52 functions, .text +0x030000; memory-card port/save flow)

| old file | markers | part |
|---|---|---|
| `save/Save_TailBlobFlow.c` | default | 1 |
| `memcard/MemCard_PortAndSaveFlow.c` | default | 1 |

#### `menu/menu_status.c` — PARTIAL (11 units, 25 functions, .text +0x033478; save background, input, stat panels)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_SaveBackground.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_SaveBgIsFadeComplete.c` | as comm -G8 | 1 |
| `item/Inv_FetchSelectedInventoryItem.c` | default | 2 |
| `menu/Menu_HandleInput.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_SetItemContext.c` | as comm -G8 | 3 |
| `menu/Menu_CheckItemAffordable.c` | cc1 -G8, as -G8 | 3 |
| `save/Save_ClassifyPlaytime.c` | default | 3 |
| `menu/Menu_DrawSaveSlotEntry.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_StatBarAnimation.c` | cc1 -G8, as comm -G8 | 3 |
| `menu/Menu_StatPanels.c` | cc1 -G8, as comm -G8 | 3 |
| `menu/Menu_StepItemSelectScreen.c` | cc1 -G8, as -G8 | 3 |

#### `menu/menu_equip.c` — PARTIAL (8 units, 23 functions, .text +0x034974; equip screen + item detail)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_EquipScreenFlow.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `menu/Menu_ItemLabel.c` | default | 1 |
| `menu/Menu_ItemDetailPanel.c` | cc1 -G8 -fno-expensive-optimizations, as -G8 | 2 |
| `menu/Menu_AmmoSpendPanel.c` | cc1 -G8, as comm -G8 | 3 |
| `menu/Menu_EquipItemSelectionFlow.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_CreateInventoryListView.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_DrawItemDetailPanel.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_InventoryPageInputHandler.c` | cc1 -G8, as -G8 | 3 |

#### `menu/menu_skill.c` — PARTIAL (7 units, 18 functions, .text +0x036B34; skill list/action, weapon mods)
Blockers: markers: 6 compatible parts

| old file | markers | part |
|---|---|---|
| `event/Evt_DeferredExec.c` | as comm -G8 | 1 |
| `menu/Menu_StepSkillList.c` | cc1 -G8, as -G8 | 2 |
| `menu/Menu_OnEquipConfirm.c` | cc1 -G8, as -G8 | 2 |
| `menu/Menu_CreateSkillActionScreen.c` | default | 3 |
| `menu/Menu_SkillActionHandler.c` | cc1 -G8, as comm -G8 | 4 |
| `menu/Menu_CreateInvSwapView.c` | cc1 -G8, as noli -G8, aspsx 2.21 | 5 |
| `menu/Menu_InventoryWeaponModFlow.c` | cc1 -G8, as comm -G8 | 6 |

#### `menu/menu_item.c` — PARTIAL (5 units, 36 functions, .text +0x037D60; item actions, save/load nav, bonus points)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_ConfigureScreen.c` | default | 1 |
| `menu/Menu_ItemActionFlow.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_StepSaveLoadScreen.c` | cc1 -G8, as -G8 | 1 |
| `menu/Menu_SaveLoadNavigate.c` | default | 2 |
| `menu/Menu_InventoryAndBonusPoints.c` | cc1 -G8, as comm -G8 | 3 |

#### `menu/menu_main.c` — BLOCKED-MARKERS (4 units, 26 functions, .text +0x03B59C; main menu view, visual settings, level-up)
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_CreateMainMenuView.c` | default | 1 |
| `menu/Menu_VisualSettingsFlow.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_LevelUpFlow.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `menu/Menu_OpenEquipSlotSwap.c` | cc1 -G8, as -G8 | 4 |

#### `menu/menu_save.c` — PARTIAL (8 units, 34 functions, .text +0x03CCB4; dialogs, save select, rename)
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_Dialogs.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_SaveSlotListView.c` | cc1 -G8, as -G8 | 1 |
| `menu/Menu_DrawSaveSlotPortLabel.c` | default | 1 |
| `menu/Menu_StepSaveSelect.c` | cc1 -G8, as noli -G8, aspsx 2.21 | 2 |
| `menu/Menu_MemCardProgress.c` | cc1 -G8, as comm -G8 | 3 |
| `menu/Menu_OpenSaveLoadEntryPoint.c` | default | 3 |
| `menu/Menu_OpenRenameScreen.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_NameEntryFlow.c` | cc1 -G4096, as -G8 | 4 |

#### `menu/menu_inventory.c` — READY (10 units, 37 functions, .text +0x03EF04; G0464 medium join (inventory screens))

| old file | markers | part |
|---|---|---|
| `menu/Menu_OpenEquipInventoryScreens.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_InventoryCategoryLists.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_InventoryPageNavHandler.c` | cc1 -G8, as -G8 | 1 |
| `menu/Menu_OpenSkillSelectionView.c` | default | 1 |
| `menu/Menu_EquipSwap.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_ItemUsePanel.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_EquipItemIcon.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_DrawListRenderers.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Menu_DrawEquipSelectionList.c` | cc1 -G8, as -G8 | 1 |
| `menu/Menu_SelectionListCallbacks.c` | cc1 -G8, as comm -G8 | 1 |

#### `menu/menu_list.c` — PARTIAL (10 units, 55 functions, .text +0x0406EC; list callbacks and row drawers)
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/Menu_DrawSaveSlotList.c` | as comm -G8 | 1 |
| `menu/Menu_ListCallbacks.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_ItemUseAction.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_DrawBlendColorOptionRow.c` | default | 2 |
| `save/SaveWriteFlow.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_DrawListRows.c` | as comm -G8 | 3 |
| `menu/Menu_DrawItemSlot.c` | as comm -G8 | 3 |
| `menu/Menu_DrawSelectableEquipSlotRow.c` | cc1 -G8, as -G8 | 4 |
| `menu/Sfx_ContextPick.c` | as comm -G8 | 4 |
| `menu/Menu_ListItemDrawers.c` | cc1 -G8, as comm -G8 | 4 |

#### `battle/battle_cmd.c` — PARTIAL (13 units, 22 functions, .text +0x041884; battle command stack, ammo, weapon lists)
Blockers: markers: 5 compatible parts · unowned rodata blob between members: rodata_00196C, rodata_0019A4

| old file | markers | part |
|---|---|---|
| `battle/BattleCmd_InitTableCursor.c` | cc1 -G8, as -G8 | 1 |
| `battle/BattleCmd_StackFlow.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_ActiveListCommand.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `battle/BattleCmd_ActiveStats.c` | default | 2 |
| `battle/BattleCmd_GetRemainingAmmo.c` | cc1 -G8, as comm -G8, FG0 | 2 |
| `battle/BattleCmd_CommitAndSyncAmmo.c` | default | 2 |
| `aya/Aya_WeaponChangeAndSpellCheck.c` | default | 2 |
| `aya/Aya_UnlockParasiteSpell.c` | cc1 -fno-schedule-insns | 3 |
| `battle/Math_IntSqrt.c` | default | 4 |
| `battle/BattleCmd_CommitAmmoAndUpdate.c` | default | 4 |
| `battle/Inv_BuildWeaponList.c` | cc1 -G8, as -G8 | 4 |
| `battle/BattleCmd_LoadWeaponModifiers.c` | cc1 -G8, as -G8 | 5 |
| `battle/Battle_GetScaledMaxHP.c` | default | 5 |

#### `aya/aya_stats.c` — PARTIAL (4 units, 19 functions, .text +0x042648; armor list + stat derivation)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `item/Inv_BonusSlotCount.c` | cc1 -G8, as -G8 | 1 |
| `battle/Inv_BuildArmorList.c` | cc1 -G0, as comm -G0 | 1 |
| `aya/Aya_StatDerivation.c` | cc1 -G0, as comm -G0 | 2 |
| `menu/Menu_SoundsAndBattleQueries.c` | default | 2 |

#### `item/inventory_list.c` — PARTIAL (16 units, 62 functions, .text +0x042F0C; active-list selection/insert/query)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `akao/Akao_SetBgmVolumeFade.c` | cc1 -G8, as -G8 | 1 |
| `menu/Menu_ActionSubmenuSelection.c` | cc1 -G8, as -G8 | 1 |
| `time/GameTime_CounterSeconds.c` | default | 1 |
| `item/Inv_DropRng.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_ActiveListSelection.c` | cc1 -G8, as comm -G8 | 1 |
| `item/inv2.c` | default | 1 |
| `item/Inv_LookupActiveListDisplayData.c` | cc1 -G8, as -G8 | 1 |
| `item/Inv_RebuildSelectionBitset.c` | default | 1 |
| `item/Inv_DataLookup.c` | cc1 -G8, as comm -G8 | 1 |
| `menu/Sfx_DrawSlotRow.c` | cc1 -G8, as -G8 | 1 |
| `item/Inv_FindFirstEmptySlot.c` | cc1 -G8, as -G8 | 1 |
| `item/Inv_InsertItem.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_CountByValue.c` | as comm -G8 | 1 |
| `item/Inv_ActiveListQueries.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_FilteredList.c` | cc1 -G8, as -G8 | 1 |
| `item/Inv_RandomPickAndCompact.c` | cc1 -G8, as comm -G8, FG0 | 2 |

#### `item/item_use.c` — PARTIAL (6 units, 41 functions, .text +0x04564C; capacity, random item, item actions)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `item/Inv_EquipmentCapacityFlow.c` | cc1 -G4, as comm -G4, FG0 | 1 |
| `item/Inv_RollRandomItemType.c` | cc1 -G8, as comm -G8 | 2 |
| `item/Item_PackParasiteSpells.c` | as comm -G8 | 2 |
| `aya/Aya_UnlockParasiteSpellById.c` | default | 2 |
| `item/Inv_PackedListItem.c` | cc1 -G8, as comm -G8 | 2 |
| `item/Inv_ItemActions.c` | cc1 -G8, as comm -G8, FG0 | 3 |

#### `item/inventory_swap.c` — PARTIAL (9 units, 59 functions, .text +0x0486CC; transfer/swap/armor capacity/sort)
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `menu/menu_get.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_TransferItem.c` | cc1 -G8, as comm -G8, FG0 | 1 |
| `item/Inv_CanAddActiveListItemToAya.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_DrawArmorItemDetail.c` | cc1 -G8, as -G8 | 2 |
| `item/Inv_InventorySwap.c` | cc1 -G8, as comm -G8 | 2 |
| `menu/Menu_OpenItemUsePanelAtIndex.c` | cc1 -G8, as comm -G8 | 2 |
| `item/Inv_ArmorCapacity.c` | cc1 -G8, as comm -G8, FG0 | 3 |
| `item/Inv_CompareItemsForSort.c` | cc1 -G8, as -G8 | 4 |
| `menu/Menu_BattleEquipMode.c` | cc1 -G8, as comm -G8 | 4 |

#### `save/save_data.c` — PARTIAL (10 units, 43 functions, .text +0x04C0A8; save header/metadata, item find)
Blockers: markers: 5 compatible parts

| old file | markers | part |
|---|---|---|
| `item/stat.c` | as div | 1 |
| `save/Save_MetadataWindow.c` | cc1 -G8, as comm -G8 | 1 |
| `item/Inv_FindItem.c` | cc1 -G8, as comm -G8 | 1 |
| `save/Save_BuildHeader.c` | default | 2 |
| `save/Save_RestoreHeader.c` | cc1 -G8, as -G8 | 3 |
| `menu/Menu_RunFrame.c` | cc1 -G8, as comm -G8 | 3 |
| `item/Inv_EquipmentSelection.c` | cc1 -G8, as comm -G8, FG0 | 4 |
| `save/Save_GetCurrentMapNumber.c` | default | 4 |
| `menu/Menu_InitScrollDirection.c` | cc1 -G8, as -G8 | 5 |
| `item/Inv_InitMaxLevelInventory.c` | cc1 -G8, as comm -G8, FG0 | 5 |

#### `text/text_table.c` — SINGLE (1 units, 17 functions, .text +0x04E28C; text/data table lookups)

| old file | markers | part |
|---|---|---|
| `table/Tbl_DataTableLookups.c` | default | 1 |

#### `menu/menu_queue.c` — SINGLE (1 units, 17 functions, .text +0x04E688; menu input queue)

| old file | markers | part |
|---|---|---|
| `menu/queue.c` | cc1 -G8, as comm -G8, FG0 | 1 |

#### `draw/draw.c` — BLOCKED-MARKERS (4 units, 59 functions, .text +0x04F050; text primitives, wipes, gradients)
Blockers: markers: 4 compatible parts

| old file | markers | part |
|---|---|---|
| `gpu/Draw_BaseOffset.c` | default | 1 |
| `gpu/Draw_TextPrimitives.c` | cc1 -G8 -fno-cse-skip-blocks, as div comm -G8 | 2 |
| `gpu/Draw_AllocColorQuad.c` | cc1 -G8 -fno-schedule-insns, as div -G8 | 3 |
| `gpu/Draw_WipeAndGradient.c` | cc1 -G8, as comm -G8 | 4 |

#### `menu/menu_widget.c` — SINGLE (1 units, 71 functions, .text +0x052D68; widget tree (71 functions))

| old file | markers | part |
|---|---|---|
| `menu/MenuWidget_Lifecycle.c` | cc1 -G8, as div comm -G8 | 1 |

#### `script/sfx_queue.c` — SINGLE (1 units, 4 functions, .text +0x055B6C; field SFX queue)

| old file | markers | part |
|---|---|---|
| `task/Task_FieldSfxQueue.c` | cc1 -G0, as comm -G8 | 1 |

#### `render/render.c` — PARTIAL (6 units, 49 functions, .text +0x055DD4; camera/viewport, mesh list, OT sort)
Blockers: markers: 5 compatible parts

| old file | markers | part |
|---|---|---|
| `render/Obj_ControlEntries.c` | cc1 -fno-force-mem, as div | 1 |
| `render/Render_CameraViewportFlow.c` | as div | 1 |
| `field/Geo_LoadMeshEntry.c` | cc1 -fno-expensive-optimizations -fno-strength-reduce | 2 |
| `render/Render_MeshListAndFade.c` | cc1 -fno-strength-reduce, as div | 3 |
| `render/Render_SortOt.c` | cc1 -G8, as -G8 | 4 |
| `render/Render_FrameUpdateAndScreenState.c` | as div | 5 |

#### `asset/asset.c` — PARTIAL (5 units, 16 functions, .text +0x05994C; TIM load/unload, display lists, voice banks)
Blockers: markers: 3 compatible parts

| old file | markers | part |
|---|---|---|
| `asset/Asset_LoadTimTextures.c` | default | 1 |
| `pm/Pm_RunBanks.c` | default | 1 |
| `asset/Asset_UnloadTableEntries.c` | cc1 -fno-force-mem | 2 |
| `render/Render_InitDisplayLists.c` | default | 3 |
| `sys/Sys_GameStateAndVoiceBanks.c` | default | 3 |

#### `scene/scene_load.c` — PARTIAL (5 units, 11 functions, .text +0x05B1E4; scene/room load)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `scene/Scene_LoadSceneData.c` | default | 1 |
| `akao/Akao_LoadVoiceBankAlt.c` | default | 1 |
| `scene/Scene_LoadRoom.c` | cc1 -G1, as -G1 | 1 |
| `scene/Scene_LoadTextureAndPlayer.c` | default | 2 |
| `scene/Scene_UpdateBgDraw.c` | default | 2 |

#### `cdrom/cdrom.c` — PARTIAL (4 units, 27 functions, .text +0x05D5A4; CD track/boot reads, scene tables)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `cdrom/CD_TrackReadFlow.c` | cc1 -G8, as comm -G8 | 1 |
| `overlay/Overlay_AudioSlotFlow.c` | default | 1 |
| `asset/Asset_SceneTablesAndSound.c` | as div | 2 |
| `cdrom/CdRom_BootReadFlow.c` | default | 2 |

#### `pm/process.c` — READY (3 units, 24 functions, .text +0x05F1A0; display init, disc table, process manager)

| old file | markers | part |
|---|---|---|
| `gpu/Gpu_DisplayInitAndStatus.c` | default | 1 |
| `boot/Boot_DiscTableLoad.c` | default | 1 |
| `pm/Pm_ProcessManager.c` | default | 1 |

#### `draw/prim_queue.c` — PARTIAL (6 units, 6 functions, .text +0x060C1C; wireframe, GPU prim queue (2 GNU-as units))
Blockers: 2 GAME_ASM (GNU-as) unit(s) must stay separate files (source policy) · markers: 3 compatible parts · probe could not test (-g3/line-sensitive): Task_GpuPackPrimColor, Math_SqrtApprox3

| old file | markers | part |
|---|---|---|
| `render/Render_DrawWireframeModel.c` | cc1 -G8, as -G8 | 1 |
| `task/Task_InitGpuHwRegs.c` | cc1 -fno-schedule-insns | 2 |
| `task/Task_GpuFlushPrimQueue.c` | default | 2 |
| `task/Task_GpuPackPrimColor.c` | GNU-as | 3 |
| `gpu/Gpu_RenderFrame.c` | cc1 -G8, as -G8 | 4 |
| `math/Math_SqrtApprox3.c` | GNU-as | 5 |

#### `scene/player_move.c` — READY (2 units, 11 functions, .text +0x061834; player movement + TIM image)

| old file | markers | part |
|---|---|---|
| `scene/Scene_PlayerMovement.c` | default | 1 |
| `asset/Asset_TimImage.c` | default | 1 |

#### `akao/spu.c` — SINGLE (1 units, 11 functions, .text +0x075884; between PsyQ objects)

| old file | markers | part |
|---|---|---|
| `akao/Spu_TransferAndLifecycle.c` | default | 1 |

#### `akao/voice.c` — SINGLE (1 units, 85 functions, .text +0x076C64; between PsyQ objects)

| old file | markers | part |
|---|---|---|
| `akao/Akao_SpuVoiceDriver.c` | default | 1 |

#### `akao/voice_bank.c` — SINGLE (1 units, 66 functions, .text +0x07A758; between PsyQ objects)

| old file | markers | part |
|---|---|---|
| `akao/Akao_VoiceBankLifecycle.c` | cc1 -fno-strength-reduce, as div | 1 |

#### `akao/sequencer.c` — SINGLE (1 units, 109 functions, .text +0x07DFC0; between PsyQ objects)

| old file | markers | part |
|---|---|---|
| `akao/Akao_SequencerFlow.c` | as div | 1 |

#### `fieldfx/object.c` — SINGLE (1 units, 14 functions, .text +0x0B2AF8; object slots + script (G1389))

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_ObjectScript.c` | default | 1 |

#### `fieldfx/sprite.c` — SINGLE (1 units, 13 functions, .text +0x0B36AC; sprite draw state (G1390))

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_SpriteRendering.c` | as div | 1 |

#### `fieldfx/collision.c` — READY (3 units, 16 functions, .text +0x0B66B0; one TU split by -fno-schedule-insns)

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_CollisionQueries.c` | cc1 -fno-strength-reduce | 1 |
| `engine/FieldEng_CollisionQueries_2.c` | default | 1 |
| `engine/FieldEng_CollisionQueries_3.c` | default | 1 |

#### `fieldfx/model.c` — SINGLE (1 units, 11 functions, .text +0x0B755C; textured model)

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_TexturedModel.c` | default | 1 |

#### `fieldfx/glow.c` — READY (4 units, 82 functions, .text +0x0B83A0; glow effect family (B/C instances))

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_GlowEffect.c` | default | 1 |
| `engine/FieldEng_GlowEffectB.c` | default | 1 |
| `engine/FieldEng_GlowEffectC.c` | default | 1 |
| `engine/FieldEng_FourLayerGlowEffect.c` | default | 1 |

#### `fieldfx/burst.c` — READY (6 units, 77 functions, .text +0x0BC4A4; burst effect family (_2/_3/B/C))

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_PointSpriteEffect.c` | default | 1 |
| `engine/FieldEng_BurstEffect.c` | default | 1 |
| `engine/FieldEng_BurstEffect_2.c` | default | 1 |
| `engine/FieldEng_BurstEffect_3.c` | default | 1 |
| `engine/FieldEng_BurstEffectB.c` | default | 1 |
| `engine/FieldEng_BurstEffectC.c` | default | 1 |

#### `fieldfx/anim.c` — SINGLE (1 units, 8 functions, .text +0x0BEC9C; field anim task list)

| old file | markers | part |
|---|---|---|
| `engine/FieldAnim_TaskList.c` | default | 1 |

#### `fieldfx/shapes.c` — READY (3 units, 47 functions, .text +0x0BF0F0; geometry, primitives, task spawn)

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_GeometryAndShapes.c` | as div | 1 |
| `engine/FieldEng_EffectPrimitives.c` | as div | 1 |
| `engine/FieldEng_TaskSpawn.c` | default | 1 |

#### `fieldfx/effects_sprite.c` — READY (8 units, 36 functions, .text +0x0C5128; sprite effects)

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_FallingSpriteEffect.c` | as div | 1 |
| `engine/FieldEng_ScatterBurstEffect.c` | as div | 1 |
| `engine/FieldEng_SpiralSpriteEffect.c` | default | 1 |
| `engine/FieldEng_FanSweepSparks.c` | default | 1 |
| `engine/FieldEng_SparkEffects.c` | as div | 1 |
| `engine/FieldEng_SpinningModel.c` | default | 1 |
| `engine/FieldEng_BallisticSprite.c` | default | 1 |
| `engine/FieldEng_AttachedEmitterEffect.c` | default | 1 |

#### `fieldfx/effects_emitter.c` — READY (10 units, 29 functions, .text +0x0CB398; emitter/flash effects)

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_WaveEmitterEffect.c` | as div | 1 |
| `engine/FieldEng_DiamondEmitter.c` | default | 1 |
| `engine/FieldEng_RadialFlashEffect.c` | default | 1 |
| `engine/FieldEng_TwinModelEffect.c` | default | 1 |
| `engine/FieldEng_DriftingSpriteEffect.c` | default | 1 |
| `engine/FieldEng_ConvergingFlashEffect.c` | default | 1 |
| `engine/FieldEng_SettlingGlowEffect.c` | as div | 1 |
| `engine/FieldEng_FadeParticle.c` | default | 1 |
| `engine/FieldEng_RotatingEffect.c` | default | 1 |
| `engine/FieldEng_PointEmitter.c` | default | 1 |

#### `fieldfx/fx_math.c` — PARTIAL (3 units, 10 functions, .text +0x0D0320; conditions + vector/angle math)
Blockers: markers: 2 compatible parts

| old file | markers | part |
|---|---|---|
| `engine/FieldEng_ObjectConditions.c` | default | 1 |
| `engine/FieldEng_VectorMath.c` | default | 1 |
| `engine/FieldEng_AngleMath.c` | cc1 -fno-schedule-insns | 2 |
