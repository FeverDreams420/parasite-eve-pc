# Architecture

The intended native target runs the original C modules, repaired in place,
with CMake and PsyZ. No new files, extracted functions or parallel game engine
are permitted while restoring the original game. The abandoned `pc/` directory and its build/test dependencies have been
removed. Do not relocate or recreate that parallel implementation.

The executable now starts `src/main/boot/main.c`, mounts and validates USA
CUE/BIN images through the original archive loader, and follows the original
startup, logo, movie and title-screen code. It contains no `pc/` sources and
needs no FFmpeg dependency. Tests compile the original modules directly;
fixtures depending on the abandoned implementation have been removed.

Optional native diagnostics stop at an explicit VBlank limit, capture VRAM and
state, and inject scheduled digital buttons into the original pad packet.
This bounded stop proves neither natural scene completion nor campaign play.
The original New Game path reaches M431/movie 2, M010 dialogue and name entry,
then M002 and M003 (the opera lobby). Headless replays check the original
player position and facing before and after twenty simulation frames of right
input, the entrance transition into the next room, and M372/movie 3 after
interaction at the opera doors. M004 dialogue 33 is covered by a bounded
replay. Further dialogue input exposes unresolved GPU list corruption.
This is not complete exploration. Frame-limit JSON
exposes original actor positions, dialogue pages, menu state and movement lock. Continue opens its card
screen after fixing a NULL dereference; saved-game loading is unverified. Real
Disc 2, full gameplay and the remaining acceptance gates are still pending.

The memory map and overlay addresses below describe the original game for
asset decoding and porting research. They are not host addresses, build
inputs or layout constraints. This project does not rebuild or byte-match
PS1 binaries.

## Native simplification policy

Gameplay fidelity is the contract. PS1 APIs may be simplified or removed; room
and scene code is linked statically with explicit state/reset, and scratchpad
work uses ordinary host buffers and the host stack. We do not emulate a CD drive.
The planned asset layer validates both CUE/BIN images once and exposes one
logical asset set, resolving source-specific IDs/offsets without disc-swap UI.
The CLI mounts images by their USA roles. Actual Disc 2 reads and transitions
remain unverified because that image is not available in the current workspace.

The maintained PsyZ source fork is
[khasinski/parasite-eve-psyz](https://github.com/khasinski/parasite-eve-psyz),
with `Xeeynamo/psyz` as upstream. Missing generic capabilities can be added
there instead of retaining unnecessary PS1 compatibility machinery. Its existing
`Psyz_VideoSetInternalResolution` scaling is used for hi-res during scene
integration. High-FPS presentation/interpolation is a later milestone; resolution
and presentation cadence must never alter simulation/gameplay.

## Shipped binaries

Parasite Eve (USA, disc 1) boots `SLUS_006.62`, a PS-X EXE. Almost everything
else the game loads comes from one large archive on the disc, `PE.IMG`,
addressed by sector. The archive holds room and scene data, textures (TIM),
AKAO music and samples, and the overlays: code blocks that the executable
reads into fixed RAM windows on demand. Overlays are not relocated; each
block is linked for one address.

The native port will load data from the archive and register room/scene code
as native modules rather than execute code loaded from the disc.

## Main executable memory map

| Range | Contents |
| --- | --- |
| file `0x000..0x800` | PS-X EXE header; load address `0x80010000`, size `0x1EE000`, entry `__SN_ENTRY_POINT` (`0x80072534`) |
| `0x80010000..0x8001220C` | read-only data: switch tables and constants, in the order of their objects |
| `0x8001220C..0x800718D0` | game code: `main`, script interpreter, actors, battle, menus, items, rendering, 2D drawing |
| `0x800718D0..0x8008D7C0` | PsyQ libraries interleaved with game-side drivers that sit beside them: CD, memory card, pad, SPU/AKAO, GPU queues |
| `0x8008D7C0..0x800910A0` | AKAO sequencer: note timing and track commands |
| `0x800910A0..0x800C20C8` | initialized data, small data and BSS (zero bytes in the image from `__bss_start` = `0x8009CDF8` to `_end`); `$gp` = `0x8009CD70`; the main state block `g_GameState` is at `0x800B0CD8` |
| `0x800C20C8..0x800E0060` | second code segment: the field effect engine (`src/main/fieldfx`, effect pool, emitters, particles) with its own rodata |
| `0x800E0060..0x801FE000` | image tail, almost entirely zero: BSS, buffers and the overlay windows |
| `0x801FFFF0` | initial stack top |

Entry is the PsyQ start-up (`psyq/libsn/snmain.c`): it clears BSS, sets
`$sp`, `$fp` and `$gp`, initializes the heap and calls `main`
(`src/main/boot/main.c`).

## Overlay windows

| Overlay | Link address | Count | What it is |
| --- | --- | --- | --- |
| `room_mNNN_<place>` | `0x8018EFE8` | 162 | code and data of one map (`m` number), section 3 of its PE.IMG record |
| `scene_eNN_<place>` | `0x8018EFE8` | 22 | special-scene sets: entry `NN` of the table at `0x80094048`, section 3 |
| `fx_field` | `0x8018EFE8` | 1 | effect block (PE.IMG sectors 10994..10995) |
| `fx_common` | `0x8018EFF0` | 1 | effect code block (PE.IMG sectors 1792..1817) |
| `mdec_diag` | `0x8010BCF8` | 1 | MDEC diagnostics and the PsyQ LIBPRESS VLC decoder with its tables |
| `menu_memcard` | `0x80120D00` | 1 | memory-card menu |
| `boot_display` | `0x80122FA4` | 1 | boot screens: text blocks, glyphs, display buffers |
| `fx_common_sector17` | `0x80170000` | 1 | one PE.IMG sector duplicating part of `fx_common` (its sector 17); data only |

Rooms, scene sets and the effect blocks are linked for the same window, so
only one of them can run at a time. `boot_display` and `menu_memcard`
contain fragments copied with their own link addresses in the PS1 runtime.
The native runtime must give these modules independent state and ownership.

Overlays call into the executable directly by address. Each overlay is linked
separately against the executable's symbols; nothing is shared between
overlays at link time, so code common to several rooms exists once per room
in the binaries. In the source the room library linked into 125 room and
scene overlays exists once (`src/overlays/room_lib/actor_classes.c`)
and is compiled into each of them; `src/overlays/room_lib/` also holds the
templates other shared code still instantiates per overlay.

## How the game runs

- **Boot.** `main` opens `PE.IMG` (`Asset_OpenPeImage`, which also identifies the
  disc), initializes the game state and subsystems, loads scene data, then
  runs a dispatch loop keyed by a scene token; the default case runs one
  frame (`Boot_RunFrame`).
- **Frame.** `Boot_RunFrame` resets the GPU pipeline and ordering tables,
  handles field state transitions, updates the entity list and actors,
  renders, steps the AKAO voice table and presents the frame at VSync.
- **Rooms.** `Scene_LoadRoom` resolves a map number, streams the map's three
  PE.IMG ranges (scratch, textures, room payload), uploads their TIM lists
  and relocates the room directory into `g_GameState`: entity banks, script
  blocks, task slots and sound banks.
- **Scripts.** Actor and scene behaviour is bytecode: the interpreter in
  `src/main/script` runs per-actor command streams, and the process manager in
  `src/main/pm` schedules script batches in slots.
- **Battle.** The ATB battle system (`src/main/battle`, `aya`, `item`) runs in
  the main executable on top of the same actor and render code.
- **Menus.** The in-game menus (`src/main/menu`) are a widget tree with its
  own input queue; the memory-card screens are the `menu_memcard` overlay.
- **Sound.** Square's AKAO driver (`src/main/akao`) sequences music and
  effects on the SPU through the PsyQ LIBSPU routines.
- **Effects.** Visual effects run as callbacks in task pools: generic ones in
  the field effect engine segment, room- and scene-specific ones in the
  overlays.

## Native build and module integration

```
existing src/main/ + src/overlays/ -> host C11 compiler -> original game + PsyZ
```

The root `CMakeLists.txt` builds the original sources for both the executable
and tests, without the abandoned parallel implementation.
There is no historical compiler, splat step, PS1 linker, assembly extraction,
retail checksum verification or decompilation report in the build.

Before adding a module, replace real MIPS/GTE operations and hardware access
with C or PsyZ, separate serialized data from host pointers, and give globals
normal definitions. Assets may contain original addresses that need explicit
decoding; they cannot be dereferenced as host pointers. Some initializers
and tables previously came from the disc and are still missing from the
standalone C source. Recover the needed data through asset loading as each
module is integrated, without rebuilding a PS1 image.

Legacy SDK implementations in `src/main/psyq/` are retained for functional
reference. PsyZ owns the host runtime; avoid duplicate implementations when
porting callers. Room/scene sources retain the original organization until
native registration and state ownership are implemented.

## Repository layout

| Path | Contents |
| --- | --- |
| `src/main/<subsystem>/` | game code to port |
| `src/main/psyq/lib*/` | legacy runtime behavior to compare with PsyZ |
| `src/overlays/` | room, scene and effect modules, plus shared room code |
| `include/` | shared types, asset layouts and legacy interfaces |
| `third_party/psyz/` | host runtime and SDL dependency |
| `docs/` | native development guidance and data-format reference |

Game data and proprietary SDK files are not distributed. The native smoke
test builds without them; full boot requires a user-supplied USA Disc 1 BIN. See [CONTRIBUTING.md](../CONTRIBUTING.md).
