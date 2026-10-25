# Third-Party Source Notices

## psy-q-decomp

`src/main/psyq/libspu/SPU_StartDmaRead.c` is adapted from
`src/spu/s_crwa.c` in [sozud/psy-q-decomp](https://github.com/sozud/psy-q-decomp),
revision `6edf9b24721ba02a53eb423b8a7497b82ba0768c`.
Local changes adapt symbols and types to the Parasite Eve codebase. This notice applies to
the adapted source, not to proprietary SDK archives or the game binary.

MIT License

Copyright (c) 2024 the psy-q-decomp contributors

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the " Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next paragraph) shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## PsyZ and SDL

The application links our source fork of [PsyZ](https://github.com/khasinski/parasite-eve-psyz)
and its pinned SDL submodule. PsyZ platform/runtime source is under MPL 2.0;
its `decomp/` directory retains its MIT notice. PsyZ's top-level LICENSE also
identifies unlicensed headers and PSYQ source. These notices remain separate
from the original game's copyright and do not relicense it.

The matching PsyZ sources are available through this repository's pinned
`third_party/psyz` submodule. SDL uses the zlib license, with additional notices
for its bundled components. Desktop packages retain the corresponding license
texts; consult each source file for any additional attribution.

## IBM Plex Sans

`src/main/boot/fonts/ibm_plex_sans.ttf.inc` embeds a subset of IBM Plex Sans Regular,
Copyright (c) 2017 IBM Corp., licensed under the SIL Open Font License, Version 1.1.
The full license text is included at `src/main/boot/fonts/OFL-IBMPlex.txt`.

## stb_truetype.h

`src/main/boot/stb_truetype.h` is from Sean Barrett's `stb` single-file public domain /
MIT dual-licensed libraries (`https://github.com/nothings/stb`).
