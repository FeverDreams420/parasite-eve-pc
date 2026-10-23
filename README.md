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

This is an incomplete port. Ordinary-input play has defeated Truebred Eve,
completed the authored EX ending and returned to the title. Cold replays from
a genuine late-EX save compare full reported state, native VRAM, the rendered
title and playback PCM between optimized normal and sanitized builds. The
current macOS CMake candidate passes all 90 registered tests in both builds
with the required USA assets. Full New Game-to-campaign-to-EX coverage on one
candidate, all content and platform/release validation remain open.
See [the port plan](docs/PC_PORT_PLAN.md) for checkpoints, reproducible inputs,
verification results and remaining work.

Controlled headless runs use the original pad, menus, battles and save flow.
Replays compare state, images, audio and newly written save bytes while
checking that the source memory-card copy remains unchanged. Sanitized builds
instrument the game and PsyZ/SDL C and Objective-C sources. These results cover
the recorded cases; complete audio/content coverage and independent PS1 image
and audio fidelity still require verification.

The game runs the original code in `src/main/` and `src/overlays/`. Repair whole
modules in their existing files for host pointers, memory, PsyZ and disc images:
no new production files, no extracting selected functions, and no parallel
feature-by-feature engine. The abandoned `pc/` implementation and its dependent
tests have been removed; do not recreate or relocate them. Native targets compile
and link the original modules directly. Required host changes belong in those
modules and our PsyZ fork.

Tests also cover original rendering, camera, actor/input, audio-queue and
resource paths, with selected USA CPU/ROM reference cases. Historical coverage
counts and earlier unported-stage notes belong to the dated evidence in the
port plan, rather than describing the current playable boundary. Artwork, video
and audio are loaded from user-supplied CUE/BIN images; there is no additional
game engine or FFmpeg dependency.

## Build and run

Use a C11 compiler, CMake 3.21 or newer and Git. PsyZ and SDL are source
submodules. Neither the game nor its tests require FFmpeg.
No historical PS1 compiler, Sony SDK or extracted game executable is needed.

```sh
git clone https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz
git -C third_party/psyz submodule update --init external/SDL
cmake -S . -B build -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
./build/parasite-eve-pc --disc "/path/to/Parasite Eve (Disc 1).bin"
```

Pass `--disc2 "/path/to/Disc 2.cue"` to keep both sources open. Images
are assigned by their authored USA disc markers; original disc-mode selection
chooses the matching mounted volume without a tray or swap prompt. Both USA
images are exercised by real-data tests, including Disc 2 rooms; complete
campaign and image-set/error coverage still require the audit in the port plan.
CUE support covers one BINARY MODE2/2352 track, relative quoted FILE paths and
INDEX 01 offsets. Duplicate identities and missing startup Disc 1 fail explicitly.

Automated runs use `--headless --frame-limit N`; this is a forced diagnostic
boundary, not a completed-game result. `--dump-vram PATH` writes all 1024×512
16-bit VRAM words at that boundary. JSON on stdout includes scene/game flags,
actors and raw 16.16 animation positions, menus, inventory, character and combat resources,
original reward lists, movie state and the active display rectangle/format.
Decode the selected rectangle as RGB555 or packed RGB24 according to
`display.rgb24`.
`--headless-gpu --internal-resolution 2` uses the actual GPU renderer with a
hidden window (multipliers 1–8). `--dump-frame PATH` writes the rendered display
as binary RGB888 PPM at each control/terminal boundary, before window/aspect
scaling. RGB24 movies remain native resolution. It requires `--frame-limit`;
use a different path from `--dump-vram`. Captures overwrite their previous
boundary output, so copy them before advancing when retaining a sequence.

`PSYZ_AUDIO_CLOCK=video` runs audio and root-counter timers from generated
samples for deterministic offline playback. `--dump-audio PATH` records those
existing mixed samples as headerless 44100 Hz stereo signed-16-bit little-endian
PCM. It requires headless mode, that clock setting, and `--frame-limit`.
Audio is streamed continuously and flushed before each control/terminal report;
use distinct capture paths. No extra samples are pulled for recording. The
original bank loader services sample-clock timer waits without advancing VBlank.

Repeat `--pad-at VBLANK:BUTTON_MASK` for deterministic digital input, using
pressed PS1 button bits (for example `10000:8` holds START; `10010:0` releases it).
Input enters the original pad packet before the original mapping/repeat logic.

For interactive automation, add `--control-at N` with a later `--frame-limit`.
At that VBlank the headless process emits state and pauses for stdin commands:
`advance VBLANK_COUNT BUTTON_MASK` or `quit`. Each advance emits another state
and, when requested, VRAM capture. The control state also reports original
combat targets, action queues and enemy ATB charge/increment. Keep stdin open
while controlling the process;
EOF exits successfully. Scheduled pad events must not extend beyond the first
control boundary.

Set `PSYZ_AUDIO_CLOCK=video` for deterministic headless replay: PsyZ opens no
audio device and advances its mixer with video VBlanks, so pausing input also
pauses the emulated audio clock. This is the clock mode used by the recent
controlled replays; it does not prove audio fidelity by itself.

Interactive keyboard events feed the original pad packets; Enter, keypad Enter
and Escape map to START, including movie skipping. The SDL GPU and OpenGL
backends present movie RGB24 as three packed VRAM bytes per pixel. Full campaign
acceptance and complete visual/audio verification remain pending.

For coverage fixtures, `--diagnostic-hp-at VBLANK:EXPECTED_HP:VALUE` schedules
an explicit health change. It requires headless mode and a frame limit covering
all events. Frames must increase strictly; a missing player, unexpected HP or
value above maximum HP aborts the replay with status 2. Each applied event is
logged as JSON, and the final record includes `coverage_only: true`,
`ordinary_campaign_proof: false`, and scheduled/applied counts. Use private
memory-card copies for these fixtures. Such runs cannot establish an ordinary
campaign playthrough. The normal game path has no scheduled health changes.

Enable `BUILD_TESTING=ON` for tests of the original modules, CPU reference
results, real assets and headless startup. Tests that depended on the abandoned
parallel implementation were removed with it; their former coverage is not
claimed by the remaining suite. Full campaign acceptance is still pending.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
PSYZ_HEADLESS=1 SDL_AUDIODRIVER=dummy \
PE_TEST_DISC="/path/to/Parasite Eve (Disc 1).cue" PE_REQUIRE_GAME_ASSETS=1 \
ctest --test-dir build --output-on-failure --no-tests=error
```

The original executable is `./build/parasite-eve-pc`; the abandoned frontend
and its `--smoke-test` option are no longer built. The `original-game-bootstrap`
test launches the actual original entrypoint with a real CUE/BIN and checks
bounded headless execution, scheduled START, New Game and Continue input,
and a deterministic opening VRAM capture.

For optional Clang/LLVM execution coverage, configure a separate build with
`-DPE_ENABLE_COVERAGE=ON -DBUILD_TESTING=ON`. This is off by default and uses
atomic counters in the game, PsyZ and SDL. CTest writes separate process profiles
to `<build>/profiles/<test-name>/<pid>.profraw`. For direct game runs, set
`LLVM_PROFILE_FILE` to a writable path containing `%p`. The headless runner
flushes its profile before successful forced exits, including `quit` and EOF;
a profile write failure exits with status 2.

Merge profiles from the same executable with `llvm-profdata merge -sparse
<profiles> -o run.profdata`, then inspect `llvm-cov report <executable>
-instr-profile=run.profdata` or export with `llvm-cov export`. On macOS these
commands are available through `xcrun`. Keep the matching executable and source
revision with each measurement. Execution counts identify exercised code;
they do not prove correctness or account for functions absent from the linked
binary. Full source and content acceptance remains a separate requirement.

## Layout

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
