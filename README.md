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

The executable runs the original game entrypoint. Recorded ordinary input
covers New Game, the first Melissa fight, exploration, subsequent bosses and
story progression through Day 3 to the NYPD. Genuine phone-created checkpoint
N in M112 also loads in the original USA game: player position, HP, all inventory
slots and equipment records agree. Ordinary input from N wins M120 and M122,
collects and uses healing items, and reaches the M123 boss. M123 has not been
won, and the remaining campaign and EX content are incomplete. Earlier runs
with explicit diagnostic HP changes are separate evidence, not proof of an
ordinary campaign completion.

The current PsyZ polygon attribute fix has 144 independent software PS1 GPU
reference cases. A recorded M115 scene now agrees with the reference across
all 16-bit VRAM bytes, with identical gameplay state before and after the fix
and matching normal/sanitized runs. This establishes that scene and pose;
complete visual fidelity and hi-res coverage remain unverified. A fresh ordinary
New Game route with the original boot clocks running reaches both theater keys,
including another corridor victory, level-up and a genuine phone Save. Normal
and sanitized runs agree with the live session in full state and VRAM. A further
ordinary replay includes the optional parrot victory and reward, finishes the
second Melissa encounter and returns to field control after dialogue 133, with
the same full-state and VRAM agreement. Its regression test also requires the
original parrot defeat flag. The original USA game also loads that Save with matching HP, position, all inventory
slots and equipment records. Recorded ordinary input also wins the M027/M028
sewer battles and traverses M031. The bootstrap replay suite passes in both
builds with the original clocks running, and a further ordinary replay wins M032.
A complete New Game replay now also opens its gate, passes the M033 Eve scene,
and reaches the M034 two-target tutorial. A further ordinary replay collects the M031/M334 chests,
uses the original equipment upgrade menu, wins M032 and creates phone Save B.
Normal/sanitized runs agree on full state, VRAM and both save payloads; a cold
Continue B also returns to field control with matching state and VRAM.
Ordinary input from B now defeats both alligator phases, collects the reward
and reaches Daniel's M036 scene. Normal/sanitized runs match the controlled
session in full state and VRAM, with the source save unchanged.
These are coverage results, not a completed-campaign result.
All 23 resident effect programs (85–107) are registered and pass full PM
lifecycle tests, including sanitized runs. Particle updates also agree with
1,344 original USA CPU/ROM cases. Actual use of every spell and effect in
the campaign remains unverified.
See [the port plan](docs/PC_PORT_PLAN.md) for current evidence and limitations.
Artwork, video and audio come from user-supplied CUE/BIN images. Full gameplay,
visual fidelity and end-to-end audio remain unfinished; isolated audio tests
do not prove complete playback.

Native targets compile original modules in place, without an additional game
engine or FFmpeg. Tests exercise original room/assets, audio, camera, actor,
input and rendering paths. The abandoned implementation and its dependent
fixtures were removed; earlier coverage claims that used those fixtures are
historical and do not describe the current suite. Full scene integration,
unsupported room descriptors and remaining hardware/address assumptions need
work.

The port must run the original game code in `src/main/` and `src/overlays/`.
Modify those sources in place for host pointers, memory, PsyZ and disc-image
access. Do not recreate gameplay feature by feature in `pc/` or introduce a
parallel engine. Repair whole original modules in their existing files: no new
files and no extracting selected functions. The abandoned `pc/` directory and its dependent tests have been removed.
Do not recreate or relocate that parallel implementation. Required host changes
belong in the existing original modules and PsyZ. `parasite-eve-original` now compiles the first original
modules directly and links them into the executable. Full integration is pending.

The original `fx_common/camera.c` now compiles whole and has native node pools,
camera/frustum math, path sampling and image-backed resource loading. Tests
compare these paths with USA code and real day/night banks. The three original
model drawers also match USA command payloads and ordering chains for all 170
day/night models in 2040 cases. The fourth drawer, including FT3/FT4 subdivision
and texture-distance selection, matches another 252 USA cases. Original fades,
markers and night shade now use native packets and shared motion state, with
headless tests of both buffers. Original camera motion/endpoints and all 79
adjacent path pairs also have native state and USA reference tests. The complete
original scene initializer now creates all 187 nodes from real image resources;
900 USA reference cases cover its ten variants and scene entry choices. Its
original controller matches 21,624 USA states, and the complete original frame
loop executes 2,664 headless frames with real textures, SDL input, playback and
demo exits. Original-program startup now reaches the title screen headlessly;
campaign integration remains unfinished.

## Build and run

Use a C11 compiler, CMake 3.21 or newer and Git. PsyZ and SDL are source
submodules. Neither the game nor its tests require FFmpeg.
No historical PS1 compiler, Sony SDK or extracted game executable is needed.

```sh
git clone https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz third_party/psyz/external/SDL
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build --parallel
./build/parasite-eve-pc --disc "/path/to/Parasite Eve (Disc 1).bin"
```

Pass `--disc2 "/path/to/Disc 2.cue"` to keep both sources open. Images
are assigned by their authored USA disc markers; original disc-mode selection
chooses the matching mounted volume without a tray or swap prompt. Only Disc 1
has been tested with real data; Disc 2 and the complete campaign remain unverified.
CUE support covers one BINARY MODE2/2352 track, relative quoted FILE paths and
INDEX 01 offsets. Duplicate identities and missing startup Disc 1 fail explicitly.

Automated runs use `--headless --frame-limit N`; this is a forced diagnostic
boundary, not a completed-game result. `--dump-vram PATH` writes all 1024×512
16-bit VRAM words at that boundary and stdout records the game flags, scene,
movie state and active display rectangle/format as JSON. Decode the selected
rectangle as RGB555 or packed RGB24 according to `display.rgb24`.
Repeat `--pad-at VBLANK:BUTTON_MASK` for deterministic digital input, using
pressed PS1 button bits (for example `10000:8` holds START; `10010:0` releases it).
Input enters the original pad packet before the original mapping/repeat logic.
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
