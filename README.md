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
a user-supplied Disc 1 BIN; menu artwork comes from `PE.IMG`.

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

Interactive mode requires a graphical session and an audio device; automated
boot tests can use `--headless`. You can set
`PE_DISC_IMAGE` instead of passing `--disc`; `make run` uses that environment
variable. Enter/Space skips the intro and opens the title options. Arrows or
W/S change the selection, Enter confirms, and Escape returns to the title
prompt. Continue remains disabled until native save support exists. Selecting
New Game or Tutorial currently exits; their gameplay is not integrated.

`make ci` builds and runs the asset-free disc-reader and automation tests. The
boot-assets and boot-replay tests skip when no image is supplied. To verify complete movie decoding and
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
than silently skipping `boot-assets` or `boot-replay`.
