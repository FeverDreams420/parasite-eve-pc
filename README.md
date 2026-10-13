# Parasite Eve PC

A native C11 PC port of the North American PlayStation release of Parasite Eve
(`SLUS-006.62`), using [PsyZ](https://github.com/Xeeynamo/psyz) for the
PlayStation runtime APIs.

The code was imported from
[parasite-eve-decomp](https://github.com/khasinski/parasite-eve-decomp) through
revision `1d20afe62`. From this point this repository develops independently
as a native C project. Byte matching, retail executable checksums, original
compiler output, fixed code addresses and PS1 binary builds are no longer
acceptance requirements. Changes are evaluated by behavior, portability,
readability and native builds.

The port is not playable yet. The executable opens a small PsyZ window,
draws a frame counter and exits after eight frames. The game sources under
`src/` are retained for porting and are not yet linked into that executable.
Remaining MIPS/GTE instructions, hardware access, address aliases and
32-bit pointer assumptions need C implementations or PsyZ adapters before
the affected modules can join the host build.

## Build and run

Use a C11 compiler, CMake 3.21 or newer and Git. PsyZ and SDL are source
submodules; no historical PS1 compiler, Sony SDK, Python packages or extracted
game executable is needed for this build.

```sh
git clone https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz third_party/psyz/external/SDL
cmake -S . -B build
cmake --build build --parallel
./build/pc/parasite-eve-pc
```

The smoke test requires a graphical session. `make`, `make run` and `make ci`
wrap the native build. `make ci` builds the current host target; it does not
verify or compile the unported game modules. A direct `cmake -S pc -B pc/build`
build remains supported.

## Layout

- `pc/` — native executable and PsyZ integration.
- `src/main/` — game logic to port; `psyq/` holds legacy runtime implementations
  whose functionality must be checked against PsyZ before replacement.
- `src/overlays/` — room, scene and effect modules to register natively.
- `include/` — game types, data formats and legacy platform interfaces.
- `third_party/psyz/` — PsyZ and its SDL dependency.
- `docs/` — native architecture, conventions, port plan and data-format notes.

See [CONTRIBUTING.md](CONTRIBUTING.md) and
[the port plan](docs/PC_PORT_PLAN.md) for the development workflow.

This repository contains source only. Disc images, game assets, FMV, memory
cards and proprietary SDK files are excluded. Running disc-backed game
features will require the user's own legal copy of the game.
