# Parasite Eve PC

A native C11 PC port of the North American PlayStation release of Parasite Eve
(`SLUS-006.62`), using [our PsyZ source fork](https://github.com/khasinski/parasite-eve-psyz) for the
PlayStation runtime APIs.

The code was imported from
[parasite-eve-decomp](https://github.com/khasinski/parasite-eve-decomp) through
revision `1d20afe62`. From this point this repository develops independently
as a native C project. Byte matching, retail executable checksums, original
compiler output, fixed code addresses and PS1 binary builds are no longer
acceptance requirements. Changes are evaluated by behavior, portability,
readability and native builds.

The native executable boots through the opening movie to the original title
screen and main menu. MDEC video and stereo XA audio are decoded directly from
a user-supplied Disc 1 CUE/BIN; menu artwork comes from `PE.IMG`.

Gameplay is not playable yet. The game sources under `src/` are retained for
porting and are not yet linked into that executable. Remaining MIPS/GTE
instructions, hardware access, address aliases and 32-bit pointer assumptions
need C implementations or PsyZ adapters before the affected modules can join
the host build.

## Build and run

Use a C11 compiler, CMake 3.21 or newer, Git, pkg-config and FFmpeg development
libraries (`libavformat`, `libavcodec`, `libavutil`, `libswscale`, `libswresample`).
On macOS install them with `brew install ffmpeg pkg-config`. PsyZ and SDL are source
submodules; no historical PS1 compiler, Sony SDK, Python packages or extracted
game executable is needed for this build.

```sh
git clone https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz third_party/psyz/external/SDL
cmake -S . -B build
cmake --build build --parallel
./build/pc/parasite-eve-pc --disc "/path/to/Parasite Eve (Disc 1).bin"
```

`--discs "/path/to/Disc 1.cue" "/path/to/Disc 2.cue"` mounts both sources
throughout boot. `pc/assets.h` keeps source provenance for identically named
files and allows reads from either image without a disc-swap operation. CUE
support currently covers a single BINARY MODE2/2352 track, relative quoted
FILE paths and INDEX 01 offsets. Other layouts fail explicitly. Disc role/version
validation and the game-wide asset catalog are still pending; this is not yet
proof of gameplay or correct Disc 2 content. `--disc` remains a boot diagnostic.

Interactive mode requires a graphical session and an audio device; automated
boot tests can use `--headless`. You can set
`PE_DISC_IMAGE` instead of passing `--disc`; `make run` uses that environment
variable. Enter/Space skips the intro and opens the title options. Arrows or
W/S change the selection, Enter confirms, and Escape returns to the title
prompt. Continue remains disabled until native save support exists. Selecting
New Game or Tutorial loads the original starting room data (M431 or M353)
and applies its scratch/texture-bank uploads to CPU-visible VRAM, then exits;
script entry/actor catalogs and the startup SPU sample bank are prepared in
`PeScene`, but scene execution and gameplay are not integrated.

`make ci` builds and runs the asset-free disc-reader, VRAM, script-format,
script-runtime, script-scheduler, SPU-bank, AKAO-sequence and automation tests. The
room-assets, boot-assets, boot-replay and boot-newgame tests skip when no image
is supplied. To verify complete movie decoding, room directory bounds and
menu input with your own image:

```sh
PE_TEST_DISC="/path/to/Parasite Eve (Disc 1).bin" ctest --test-dir build --output-on-failure
```

The optional PsyZ window test is `./build/pc/parasite-eve-pc --smoke-test`.
A direct `cmake -S pc -B pc/build` build remains supported.

## Layout

- `pc/` — native executable and PsyZ integration.
- `src/main/` — game logic to port; `psyq/` holds legacy runtime implementations
  whose gameplay-relevant behavior guides native replacement or removal.
- `src/overlays/` — room, scene and effect modules to register natively.
- `include/` — game types, data formats and legacy platform interfaces.
- `third_party/psyz/` — our maintained PsyZ source fork and its SDL dependency.
- `docs/` — native architecture, conventions, port plan and data-format notes.

See [CONTRIBUTING.md](CONTRIBUTING.md) and
[the port plan](docs/PC_PORT_PLAN.md) for the development workflow.

This repository contains source only. Disc images, game assets, FMV, memory
cards and proprietary SDK files are excluded. Running disc-backed game
features will require the user's own legal copy of the game.

## Diagnostic commands

`--inspect-disc`, `--decode-intro`, `--play-intro` and `--menu` each take a
Disc 1 BIN path. They inspect boot assets, decode without presentation, play
only the intro, or start directly at the title menu. Normal boot uses `--disc`.

An ordinary 2048-byte ISO preserves data files but loses the XA sectors needed
for movie audio. Use the original single-track MODE2/2352 BIN for full boot.
No game assets are copied into this repository.

The full-game roadmap favors gameplay fidelity and native simplification: PS1
APIs can be removed, room modules linked statically, and scratchpad work moved
to normal buffers. Both user-supplied CUE/BIN images will form one logical asset
set, without a physical drive or manual disc swapping. That combined layer is
planned; the current boot command still takes Disc 1 only. Hi-res uses PsyZ's
existing rendering support during game integration; high FPS is a later stage.

## Headless automation

The boot frontend can run without a visible window or audio device, using a
software renderer and dummy audio. Replays react to actual presented states,
not timed key presses, and write machine-readable JSONL events:

```sh
./build/pc/parasite-eve-pc --headless \
  --replay pc/tests/replays/boot-tutorial.replay \
  --trace build/boot.jsonl --replay-timeout-ms 5000 \
  --disc "/path/to/Parasite Eve (Disc 1).bin"
```

Each replay line is `event expected_integer action`. Actions currently cover
the title inputs (`return`, `space`, `escape`, `up`, `down`, `z`), `close`, and
`assert`. A mismatched value, incomplete replay, trace failure or timeout returns
failure. Trace values include the displayed movie-frame index, menu selection
and exit result; timestamps stay monotonic through SDL shutdown.

This interface currently covers boot/title state. Complete game-state snapshots,
input actions, RNG/clock control, scenario checkpoints and save fixture loading
will be added with `game_core`; the gameplay port remains incomplete.
`--headless` does not emulate the PsyZ GPU smoke test.

For mandatory asset tests, set both `PE_TEST_DISC` and
`PE_REQUIRE_GAME_ASSETS=1` before running CTest. Missing data then fails rather
than silently skipping tests that require game assets.

`room-assets` walks all 450 USA Disc 1 room table slots, including the final
zero slot: 414 contain loadable scratch/texture/room blobs and 36 are empty. It checks directory bounds and
transactional loading and texture upload bounds for both initial banks
(62,002,432 uploaded 16-bit words across the 414 rooms). `vram` checks upload
order, secondary image offsets, zero-height rules and atomic rejection of bad
batches without game data. This does not verify all payload semantics, render
rooms, execute scripts or prove that maps are playable. The asset-free reader
test also checks malformed room headers/TIM tables without disc images.

`pc/script.c` preserves self-relative script offsets and decodes the original
operand modes, including the tail word for arguments after the fifth. Disc 1
tests validate 2,411 entry offsets and 594 initial actor descriptors, and decode
each initial actor's first command. These checks do not traverse every branch
or execute scripts. The active starting actors of M431/M353 begin with opcode
234 (`Akao_ProcessCommand`), whose native execution remains pending.

`pc/spu_bank.c` uploads the original AKAO sample-bank format to CPU-visible
512 KiB SPU RAM and decodes instrument addresses, ADSR fields and pitches.
The startup bank comes from the executable's PE.IMG range table, with 28,176
sample bytes and 256 instrument records. Uploads and scene preparation reject
bad data before replacing existing state. Room changes preserve existing SPU
and VRAM contents before texture uploads. This is resource preparation;
sequence execution, synthesis, mixing and room-specific audio-bank changes
remain pending. `spu-bank` tests malformed/truncated banks and partial uploads,
while `room-assets` tests real startup data and scene ownership/rollback.

`pc/script_vm.c` executes one native script task with shared variable/operand
banks and writable literal operands. It supports assignments, all 24 expression
operations, conditional/unconditional jumps, wait ticks, event cursors, scene
flags, node metadata and defined
no-ops. Signed comparisons and fixed-point operations preserve the original
word semantics without C overflow/shift undefined behavior. Unsupported
opcodes and instruction-limit exhaustion return explicit errors. Tests execute
both event branches of the complete actual M431 entry-zero task; the active scene
actor still fails explicitly at AKAO opcode 234. The task runtime is compiled
into the native target but is not yet driven by the frontend's scene loop.
Subsystem dispatch supports continue, yield, retry and explicit errors, with
operand reads/writes through the original binding. Tests check live operands
between retry ticks and the original battle/actor pause gates. Audio/render
handlers, physical actor creation and full scenes remain pending;
these tests do not prove campaign execution.

`pc/script_scheduler.c` implements the original 72-node pool, three actor task
lists, child spawning, 16-bit sequence IDs and deferred collection. Tasks share
actor locals/flags/event cursors rather than copying them into independent
nodes. Tests cover insertion order within a tick, actor deletion after callback
processing, pool exhaustion/reuse and invalid links. `PeScene` owns the initial
script actor bindings and scheduler, and preserves script banks across room
changes. The frontend prepares these bindings but does not yet tick the scene;
models, movement, collision and rendering still need integration. The real
initial M431 actor reports missing AKAO opcode 234 through the scheduler.

`pc/akao_sequence.c` reads room music records and validates AKAO sequence
headers, 24-voice masks and channel offsets relative to the word following each
offset. Disc 1 contains 600 room music records: 599 validate as full sequences;
one ID-zero short record remains explicitly unresolved and has no parsed
sequence. This is metadata/offset validation, not sequence command execution
or audio output. The unresolved record is retained for further porting.

The native catalog also decodes the 250 audio-bank ranges from the supplied
executable. `pe_audio_bank_read` reads complete banks from PE.IMG with bounds
checks. `pe_spu_bank_upload_rebased` preserves the source bytes while relocating
instrument start/loop addresses and uploading to a selected SPU/instrument
bank. Tests load all 51 distinct sample banks referenced by Disc 1 room music
records. Automatic selection between active voice banks and live playback are
still pending; these tests upload data without synthesizing it.
