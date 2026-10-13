# Parasite Eve PC

Native PC port of the North American PlayStation release of Parasite Eve
(`SLUS-006.62`). The game code in this tree is the matching decompilation from
[parasite-eve-decomp](https://github.com/khasinski/parasite-eve-decomp). The
PlayStation runtime is replaced by [PsyZ](https://github.com/Xeeynamo/psyz),
a native PSY-Q stand-in. This is not an emulator and not a static recompiler.

The port is not playable yet. `pc/` builds a small PsyZ window so the SDK,
headers, and host link are in place. The decompiled game is not linked into
that executable.

This repository is source-only. It does not contain disc images, extracted
assets, FMV, memory cards, or Sony PSY-Q SDK files. A legal copy of the game
is required before any disc-backed path can run.

## Build the host smoke test

The smoke test opens a 256x240 window through PsyZ, draws a frame counter, and
exits after a few frames. It does not load `PE.IMG`.

```sh
git clone --recurse-submodules https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz third_party/psyz/external/SDL
cmake -S pc -B pc/build
cmake --build pc/build
./pc/build/parasite-eve-pc
```

![functions](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhasinski%2Fparasite-eve-decomp%2Fmain%2Fdocs%2Fbadges%2Ffunctions.json)
![code](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhasinski%2Fparasite-eve-decomp%2Fmain%2Fdocs%2Fbadges%2Fcode.json)
![PsyQ debt](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhasinski%2Fparasite-eve-decomp%2Fmain%2Fdocs%2Fbadges%2Fdebt-psyq.json%3Fv%3D5)
![Game debt](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhasinski%2Fparasite-eve-decomp%2Fmain%2Fdocs%2Fbadges%2Fdebt-game.json%3Fv%3D5)
![Overlays debt](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhasinski%2Fparasite-eve-decomp%2Fmain%2Fdocs%2Fbadges%2Fdebt-overlays.json%3Fv%3D5)

PsyZ's other submodules (`nugget`, `cimgui`) are only needed for PlayStation
cross-builds and the imgui sample. The PC target needs `external/SDL`.

## Layout

- `src/`, `include/`, `configs/` — the matching decompilation. The PlayStation
  build (`make verify`) still targets the retail executable and is unchanged.
- `third_party/psyz/` — PsyZ, as a submodule.
- `pc/` — the native CMake target.

## What has to change before the game links

PsyZ's [porting guide](https://github.com/Xeeynamo/psyz/blob/main/PORTING_GUIDE.md)
is the contract. The decomp still targets GCC 2.7 and a fixed PlayStation
address map, so a host link has to deal with at least:

- ordering tables stored as `u_long*`, which PsyZ replaces with `OT_TYPE*`
- pointers kept in `s32` / `u32`, which truncate on 64-bit hosts
- symbols that overlap in the PlayStation memory map
- hardcoded addresses such as `0x800A1234` and `0x1F800000`
- `register` asm constraints and empty scheduling barriers kept for the match
- GTE and GPU operations written as `.word` encodings rather than PSY-Q calls

The disc check that decides CD1 versus CD2 lives in `OpenPeImage`
(`src/main/render/Render_InitDisplayLists.c`). On PC that search has to read
the inserted disc image instead of `DsSearchFile` against the drive.
