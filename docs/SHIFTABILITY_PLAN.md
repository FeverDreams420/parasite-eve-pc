# Shiftability plan, phase 3: main's data

Goal: no fixed program addresses in the main executable, so that code and
data can grow, shrink or move without breaking anything, while the retail
layout keeps building byte-identical. Phases 1 and 2 (code addresses,
shadowing pins, overlay `undefined_extra`) are described in
[CODE_ORGANIZATION.md](CODE_ORGANIZATION.md#shiftability). This document is
the analysis and the work plan for phase 3: main's data.

Measured on 2026-10-13 with `make build shift-audit shift-test` (and the
`--json` outputs), after the pilot and layout commits described in sections 3
and 7.

Status (wave 4a, remnants and catalogue, section 9): no word of main refers
to a pinned program symbol any more (pinned words 4 to 0, gp-relative words
to pins 6 to 0, relocations to absolute program symbols 10 to 0). The linked
symbol file holds only hardware, kernel and relative aliases, each in an
owner block; the 1224 names nothing links against are an unlinked catalogue.

## Key conclusions

1. **Retail's data layout is the PsyQ link model, and it is recoverable from
   evidence.** The startup (`__SN_ENTRY_POINT`) clears BSS from `0x8009CDF8`
   to `0x800C20C8` and sets `$gp` to `0x8009CD70`, the first word of `.sdata`.
   The referencing units of the labels in between follow link order in
   runs: `.sdata` by object, `.sbss` by object, a small COMMON pool, `.bss`
   by object, a large COMMON pool. That tells, for every variable, which unit
   owned it and whether it was file-static (object runs) or a tentative
   global (pools). Section 1.
2. **Most pins can become linker-placed without touching code generation.**
   Every referenced data pin except the PsyQ CD state block (and one word in
   `field/actor_collision`'s `.data`) points into an asm blob. Naming the address in `configs/USA/sym.main.txt` turns the pin into
   a blob label, which the linker moves with its blob (stage A). Owning the
   variable in a C unit (stage B) is a second, per-unit step.
3. **`$gp` and the field engine no longer have fixed addresses** (done in this
   branch): `_gp = _ramsize` and `.field_engine` follows `main_VRAM_END`. A
   padded relink now links with no fixed placement left. The gp window is
   positive-only and the whole small-data block is 0x59C bytes, so it has
   32 KiB of room; an overflow fails the link (`R_MIPS_GPREL16`), it cannot
   silently mislink.
4. **Main cannot grow its globals today**: a tentative definition that no pin
   or label resolves is discarded with `.scommon`/`COMMON` and the link fails.
   The manifest needs a COMMON catch-all and in-image BSS before new data can
   be added. Section 3.5.
5. **The disc embeds no main data address.** A scan of `PE.IMG` for 32-bit
   words equal to a main symbol address finds 5 isolated (random) hits and
   none inside the 190 overlay slices. What is fixed by the disc is the load
   address, the overlay windows and the overlays' own code, which is rebuilt
   from source but shipped prelinked in `PE.IMG`. Section 6.
6. **Pilot** (`script/sfx_queue`, 3 pins): done end to end, byte-identical,
   all checks green. Pinned address words 6498 to 6443, gp-relative words to
   pinned symbols 1265 to 1243, no fixed placement left in the linker script.

## 1. Retail data layout

Section `.main` runs from 0x80010000 to the end of main's BSS; the field
engine group follows; the image (text size 0x1EE000) then covers the heap and
the overlay windows up to 0x801FE000.

| range | group | what decides the owner | evidence |
| --- | --- | --- | --- |
| 0x80010000..0x8001220C | `.rodata`, by object | link order | manifest, already linker-placed |
| 0x8001220C..0x800910A0 | `.text`, by object | link order | all C, linker-placed |
| 0x800910A0..0x8009CD70 | `.data`, by object (game, then PsyQ) | link order; several tables are shared | blobs `dtail_gp_pre_s016`, `_post_sprintf`, `_post_bios`, `_post_dsfile` and C `.data` |
| 0x8009CD70..0x8009CDF8 | `.sdata`, by object | link order (`_ramsize`, `_stacksize` first) | `_gp`; startup reads `_ramsize`/`_stacksize`; mostly initialised words |
| 0x8009CDF8..0x8009D1A0 | `.sbss`, by object | link order, monotonic from `node_pool` (1) to `widget` (63) | startup BSS clear starts here (`D_8009CDF8`) |
| 0x8009D1A0..0x8009D30C | small COMMON pool | none: order is not link order | users jump back and forth between unit 0 and unit 322 |
| 0x8009D30C..~0x800A5C00 | `.bss`, by object | link order, monotonic from `node_pool` (1) to `waitrc2` (291) | e.g. `g_FieldSfxQueue` sits between `widget` and LIBGPU state |
| ~0x800A5C00..0x800C20C8 | large COMMON pool | none | `g_GameState`, `g_AkaoCmd`, `g_SaveRuntimeState`, PsyQ `StRingSize` |
| 0x800C20C8 | `_end` | startup BSS clear stops here and `InitHeap(_end + 4, ...)` | `__SN_ENTRY_POINT` |
| 0x800C20C8..0x800E21A4 | field engine rodata, text, data | its own link order | engine strings start exactly at `_end` |
| 0x800E21A4..~0x800F3500 | field engine BSS (referenced by main) | | `fieldfx/burst` statics, `g_FieldFxAltPaletteFlag` ... |
| ~0x800F3500..0x8010BCF8 | zero, no label referenced by main | unknown: possibly runtime buffers | needs a RAM trace before it is used as growth room |
| 0x8010BCF8..0x801FE000 | overlay windows and buffers at fixed addresses | disc ABI | section 6 |

Two corrections to the current manifest follow from this:

- The `field_engine` segment starts at 0x800C1CA0, but 0x800C1CA0..0x800C20C8
  (0x428 zero bytes: `g_WayneStorageItems`, `g_InvItemArray`,
  `g_CursorRenderDataBlock`, `StRingSize`) is the tail of main's large COMMON
  pool. The segment boundary should move to file offset 0xB28C8 (vram
  0x800C20C8) and those bytes become a main data blob. Byte-identical; it
  makes `main_VRAM_END` equal retail's `_end`.
- The object runs are where "file-static" globals live. A variable in an
  object run that several of our units reference is evidence that those units
  were one retail object (or that the variable was a defined global); the
  object-run order is a TU-boundary oracle in the same way text order is.

The run boundaries above come from the referencing units of every label
(scratch script over the relocations of a `--emit-relocs` relink). The end of
the `.bss` object run lies between 0x800A5B70 and 0x800A5D50; the first agent
working that region pins it down.

## 2. Taxonomy of pinned main symbols

`relocations resolving to absolute symbols` (shift audit (c)): 7686 address
relocations to 499 distinct program-range symbols, plus 28 hardware/kernel.
The shift test sees 6443 pinned address words and 1243 gp-relative offsets to
pinned symbols. By retail group and by what the pin is today:

| group | symbols | address words | gp-relative | pin kinds today |
| --- | ---: | ---: | ---: | --- |
| function | 18 | 21 | 0 | 18 function named elsewhere |
| rodata | 2 | 8 | 0 | `jtbl_80010000` (constructor table base in `__main`), 1 blob |
| `.data` | 136 | 1518 | 0 | 94 in asm blobs, 40 inside `psyq/libcd/bios_4.c`'s data, 2 shadowing |
| `.sdata` | 6 | 204 | 11 | 3 COMMON pins, 3 blob |
| `.sbss` object run | 102 | 40 | 928 | 99 COMMON pins, 3 blob |
| small COMMON pool | 67 | 1566 | 304 | 34 COMMON pins, 32 blob, 1 C unit |
| `.bss` object run | 60 | 1016 | 0 | 58 blob (named elsewhere), 2 unlabelled |
| large COMMON pool | 104 | 1984 | 0 | 101 blob, 3 inside the field engine blob |
| field engine | 4 | 86 | 0 | 3 unlabelled (`fieldfx/burst`), 1 blob |

"Blob" means the address lies inside a splat asm data blob that already has a
label there (usually `D_xxxxxxxx`, 41 times a descriptive name that differs
from the pinned name); "COMMON pin" means a C unit holds a tentative
definition and the linker script pins it. 26 addresses carry more than one
pinned name (struct base and first field, or deliberate record views such as
`g_ActiveActor`/`g_ActiveActorRecord`).

What each kind needs:

| kind | count | stage A (shiftable) | stage B (owned) |
| --- | ---: | --- | --- |
| function named elsewhere | 18 | name the function in `sym.main.txt` (the blob word then says `Script_GetCurrentEntityFieldD`), or rename the C reference; drop the pin | done by stage A |
| shadowing pin (`g_RoomFloorY`, `g_DsReadStatusBlock`) | 2 | delete the pin, the object already defines the name at the same address | |
| initialised `.data` in an asm blob | 94 | `sym.main.txt` name for the blob label, drop the pin | carve the blob at the unit's `.data` boundaries, define the table in C (`.data` subsegment); needs the table's type, risk is in initialisers and alignment |
| data inside a C unit (`bios_4.c` CD state block) | 40 | relative linker alias `D_8009AFB8 = g_CdBiosState + 4;` (moves with the owner), or a member access where codegen does not change | define the aliases in `bios_4.c`, or members |
| `.sdata` | 6 | blob label | `.sdata` subsegment of the owner; needs the unit to compile with cc1 `-G8` (see the pilot: a forced-G0 unit emits the definition into `.data`) |
| `.sbss` object run | 102 | blob label; a COMMON then resolves to it (as `g_InvActiveListItems` already does) | owner's `.sbss` subsegment; needs MASPSX to emit a global `.sbss` label for a `.comm` (today it emits a local one, which is why units use `--use-comm-section`) |
| small COMMON pool | 67 | blob label | one pool unit defining the variables in retail order, or keep the labelled blob (see 3.5) |
| `.bss` object run | 60 | blob label | file-static in the owner, `.bss` subsegment with `linker_section_order: .data` (the pilot) |
| large COMMON pool | 104 | blob label | as the small pool |
| field engine BSS | 4 | blob label (`sym.main.txt`; three have no label at all yet) | engine units' `.bss` |
| hardware I/O, scratchpad | 10 + 6 | unavoidable | |
| kernel/BIOS RAM (`0x80000000` vectors, `0xA000xxxx`, `0xDFFC`) | 5 | unavoidable (BIOS contract) | |
| unreferenced assignments | 1549 | no object uses them, they do not affect the shift; move useful names to a symbol catalogue (many are overlay-window and cheat names, e.g. `g_HSFuse2Tx`) | |

Not pins but still fixed, from the shift test:

- 15 literal data words: two overlay-window pointers in `rodata_001E0C`
  (`0x8018EFF0` string directory buffer, `0x801ED7F8` task program buffer; ABI
  values, should be named ABI constants), the LIBCD hardware state block in
  `bios_4.c` (6 words pointing into itself and at `g_CdLastCom`), and 7 words
  in the `dtail` blobs (`0x80010EA0`, `0x80010F58`, `0x8001170C`,
  `0x800117AC`, `0x80010804` are rodata pointers splat did not relocate,
  `0x80017FFF` twice is probably two halfwords). Fix: relabel the blob words
  as symbol references (`rel.main.txt` or a symbol at the target).
- 8 literal `lui` halves: `Draw_InitGpuHwRegs` and `Draw_FlushPrimQueue`
  (the register block at 0x80070Dxx, 6), `Render_LoadFontGlyphAlt` (1),
  `DsReadCallback` (1). Fix in C: refer to the symbol.
- 13 integer-to-pointer casts of program addresses in `src/main` (`bios_4.c`
  6, `gpu_regs.c` 3, `prim_queue.c` 3, `dsread_2.c` 1), the same items seen
  from the source side.
- Header literals: initial PC `0x80072534` and text size `0x1EE000`.

## 3. Mechanisms

### 3.1 Stage A: a pin becomes a blob label

A splat data blob is one input section. The linker places it after its
predecessor, so every label inside it moves with it. Naming the address in
`configs/USA/sym.main.txt` makes splat emit `dlabel <name>` in the blob at
the next `make split`; references then relocate against that label and the
absolute assignment is deleted. This is already the project's practice
(`e5291685f`, `57b48b333`). A tentative COMMON definition in a C unit
resolves to the blob's definition; ld prints an alignment note when the
COMMON wants more alignment than the label declares, as it already does for
`g_InvActiveListItems` and nine others.

What stage A does not give: the blob's internal layout is still fixed, so
data cannot be added inside a blob, and the owner is still asm. It is enough
for code to grow and for whole data regions to move.

One address, one name. Renaming a blob label `D_8009D20C` to
`g_FieldActorListHead` removes the old name, and any C that still says
`D_8009D20C` is then silently pinned again by
`gen_undefined_addr_aliases.py` (it pins address-form names). The recipe
checks for that. When two names are deliberate (record views used to steer
GCC), renaming the C may change code; then the second name becomes a relative
alias in the linker symbol file (`g_ActiveActorRecord = g_ActiveActor;`),
which moves with its target and is not a pin. The audit counts these as
`symbolic_alias`; they are allowed migration state, not a goal.

### 3.2 Stage B: a C unit owns the variable

The unit defines the variable and the manifest places the unit's section at
the retail position inside the data stream:

```yaml
      - [0x8DA58, data, main/dtail_gp_post_collision_actor]
      - {start: 0x93980, type: .bss, name: script/sfx_queue, linker_section_order: .data}
      - [0x93AD0, data, main/dtail_gp_post_sfx_queue]
```

`linker_section_order: .data` keeps the unit's `.bss` in the `.data` list, in
the PROGBITS `.main` output section, so ld zero-fills it in the file exactly
where retail has zeros. `gen_expected.py` counts such in-image `.bss` rows
toward coverage (`inline_bss_bytes`, added with the pilot).

Which form of definition reproduces the retail section:

| retail group | C form | MASPSX/cc1 path |
| --- | --- | --- |
| `.bss` object run | `static T x[N];` | cc1 `.lcomm`, MASPSX emits a `.bss` label (global) in declaration order |
| `.sbss` object run | `static T x;` in a `-G8` unit | cc1 `.lcomm`, MASPSX `.sbss` label, local; a global needs the MASPSX change in section 8 |
| `.data` object run | `T x[] = {...};` | `.data` |
| `.sdata` object run | `T x = v;` with cc1 `-G8` | cc1 emits `.sdata`; under `MASPSX_FORCE_G0` cc1 runs at `-G0` and emits `.data` instead |
| COMMON pools | not reproducible by section order | keep the labelled blob, or one pool unit (section 3.5) |

A unit owning several variables in one group must declare them in retail
address order and match retail's padding (`SUBALIGN(4)` aligns each input
section to 4; MASPSX aligns `.sbss` items by size).

### 3.3 `$gp` (done)

`_gp` is set by splat's `ld_gp_expression: _ramsize`. Retail's `$gp` is the
start of the `.sdata` group and its first word is the startup's `_ramsize`
(0x00200000), followed by `_stacksize` (0x2000); both are now named and
`snmain.c` reads them by name. `gp_value` stays for the retail disassembly.

All gp-relative targets lie in 0x8009CD70..0x8009D30C, positive offsets only
(0x0..0x59C). When data before `.sdata` grows, `_gp` and its targets move
together and every `%gp_rel` offset is unchanged (the shift test reports
2314 unchanged gp-relative words). When small data is added inside the block,
the offsets after it change and are relocated. The window has 32 KiB of
positive room (64 KiB if `_gp` were ever re-centred, which would change every
offset and is not needed). Out-of-range offsets are a link error. The 1243
gp-relative words that still go to pinned symbols disappear with stage A of
the `.sbss`, small-pool and `.sdata` symbols; until then a shift that moves
`_gp` breaks exactly those words, which the shift test lists.

### 3.4 `.field_engine` (done) and the main/engine boundary

The `field_engine` segment has `vram_symbol: main_VRAM_END`, so the linker
script places it after main. Next step: move its start to 0x800C20C8
(section 1), so that `main_VRAM_END` is retail's `_end`.

The field engine is linked in the same program (main calls `FieldFx_*`
directly and the engine reads main's data), so it shifts with main. Its BSS
and whatever lies above it are in the `dtail_0D0860` blob up to 0x801FE000,
which also holds the overlay windows. That blob must be split into
engine data/BSS (moves), a free gap, and the overlay window block anchored at
its ABI address (`. = 0x8010BCF8` after an `ASSERT(. <= 0x8010BCF8, ...)`).
The gap between the last main-referenced engine label (~0x800F3500) and
0x8010BCF8 is about 98 KiB of zeros, but before it is used as growth room a
RAM trace has to show that nothing uses it at run time.

### 3.5 COMMON, BSS in the image, and growth

Today every tentative definition is resolved by a pin or a blob label. A new
one is not: ld discards `.scommon`/`COMMON` through `/DISCARD/ : { *(*) }`
and the link fails (`g_MenuWidgetSavedNode ... defined in discarded section
.scommon`, tried in a scratch relink). For growth, the manifest needs:

- a catch-all `*(.scommon) *(COMMON)` after the last main data blob (empty
  for the retail layout, so byte-identical);
- BSS inside the file image. Retail's BSS is zero bytes inside the PS-X EXE.
  If `.main_bss` (NOLOAD) ever becomes non-empty, `__romPos` does not advance
  and the field engine's LMA and VMA diverge. Either keep every BSS input
  in `.main` (as `linker_section_order: .data` does) or advance `__romPos` by
  `SIZEOF(.main_bss)`. Splat 0.50's `ld_bss_contains_common` only writes
  `COMMON` for NOLOAD BSS and its `ld_bss_is_noload: False` drops it, so this
  needs either a splat option or a small checked post-pass over `main.ld`.

The COMMON pools are not in link order, and GNU ld allocates COMMON in hash
order, so a pool cannot be reproduced by placing each unit's `COMMON`. Two
ways to own a pool: keep it as a labelled blob (shiftable as a block; new
globals go to the catch-all), or one pool unit that defines the pool's
variables as real definitions in retail order, with the other units keeping
their tentative definitions. The second needs the MASPSX global-`.sbss`
behaviour for the small pool and is a later decision (section 8).

### 3.6 Startup and header symbols

`snmain.c` (asm kept in the PsyQ macro) refers to the BSS bounds through blob
labels (`D_8009CDF8`, `D_800C20C8`) and to the heap words `g_SnProgramEnd`,
`g_SnHeapSize`. They move with their blobs today. Once `.sbss` is owned and the
engine boundary moves, they become linker symbols: BSS start = first `.sbss`
input of main, `_end` = `main_VRAM_END`. The PS-X EXE header's initial PC
is `__SN_ENTRY_POINT` and its text size `PSX_EXE_TEXT_SIZE`, both written by
the link (`ld_postpass.py` rewrites the two header words); a literal entry
point made the BIOS jump into the middle of the code below the startup of a
shifted build (`shift_full_test.py` now fails when the entry point does not
move with the pad).

### 3.7 Overlays

Overlays link against retail addresses (`undefined_*_auto` and
`undefined_extra` from splat and `sym.main.txt`): 389 distinct main
addresses. A shifted main therefore needs the step in the existing removal
order: link each overlay against the built main ELF (`--just-symbols` or
assignments generated from `build/USA/main.map`), and repack the rebuilt
overlays into `PE.IMG` (same sizes, patched in place at their sectors; the
ISO target currently reuses the retail `PE.IMG`). Until then a shifted main
is shiftable source but not a playable disc.

The repack exists: `tools/scripts/repack_pe_img.py` (`make pe-img-repack`,
after `make overlay-build-all`) copies `PE.IMG` and overwrites each overlay's
slice (the `dd` comment of its config) with the built binary, zero padded;
an overlay larger than its slice fails with its excess, overlapping slices
(`fx_common_sector17` in `fx_common`) must agree, and the report gives the
slack per slice. `make pe-img-check` compares the result with the retail
SHA-1 (`PE_IMG_SHA`); the retail build gives back the original image. Every
retail slice is full (slack 0), so today an overlay can change but not grow.

A shifted repack (the window block at another base) has two more cases.
`fx_common_sector17` is a byte copy of one `fx_common` sector, built as
literal data: the disc holds one copy, so the relinked `fx_common` bytes are
written and the difference is a warning. And data outside every slice can
hold addresses inside the block: assets read into a window next to an
overlay. `linkers/USA/pe_img_pointers.txt` lists the ones proven so far (the
second video player's file table read with asset[70], the credits line
table of asset[71]); `repack_pe_img.py` moves each listed word by the
block's move (`overlay_windows_VRAM` of `--main-elf` minus the retail base,
zero for the retail build). The list is evidence-based, not a scan: the
image holds about 15000 block-shaped words outside the slices, most of them
not addresses.

Sizes recorded outside the overlays. Three main tables describe the whole
image, contiguously: the u16 boundaries at `D_800930D8` (sectors 0..2224),
`g_SceneSectorDirectory` (base 2224 plus u16 offsets, to 8945) and the room
table `D_80093378` (8-byte rows per map: start sector and three sector
counts, scratch, texture and room section, to 100690, the image end). The
tool reads them from `build/USA/main.elf` and checks the coverage. Of the 190
slices, 163 are exactly one range, whose count is then the overlay's size:
the 162 rooms (room section of their map) and `mdec_diag` (boundaries 67..68).
24 open a larger range: the 22 scene sets and `fx_field` (room section of
maps 411..437 and 12, followed by more data) and `fx_common` (boundaries
75..76, read whole into the string-directory window). `boot_display` lies
inside boundaries 71..72 (read whole to 0x80120D00) and `fx_common_sector17`
inside 75..76; `menu_memcard` covers boundaries 68..70 in part, of which the
loader reads only 68..69 as code. Inside `PE.IMG`, every room section opens
with two words that its overlay holds as literal data: the payload size
(word 0, which the room table's count must hold: `ceil(size / 2048)` equals
it for all 185 room-table overlays) and the directory offset (word 1). The
tool checks both relations for the built overlays. Growing an overlay past
its slice therefore needs (a) the header words expressed as symbols in the
overlay source, (b) the room-table count and every later start sector, or
the boundary entries after the slice, recomputed; for the room table that is
a pure function of the section sizes (`count = ceil(size / 2048)`, starts
cumulative), but it moves every later sector and changes main's data, so it
is a relayout of the image and a generated table in main, not an in-place
repack, and is not implemented.

## 4. Byte identity and the recipe

Every step keeps the retail layout byte-identical: the manifest reproduces
retail addresses, the source no longer states them. A step is accepted only
if all of `make check`, `make overlay-build-all overlay-check-all`,
`make organization-check source-policy-check debt-check` and `make report`
pass, no new pin, barrier, `volatile`, asm or `goto` appears, and the
Levenshtein distance of every touched function stays 0. Run `make split`
after any symbol-file or manifest change.

### Recipe: one symbol, end to end

Worked on `g_FieldSfxQueue` (bss object run), `g_FieldSfxQueueCount`
(`.sdata`) and `g_FieldActorListHead` (small COMMON pool).

1. Find the symbol and its group: `shift_test.py -v 400` lists pinned words
   per object and symbol; the address gives the group (section 1). Look up
   the label splat already has there (`build/USA/main.map`) and every C name
   used for the address (`grep -rn D_8009D20C src/main include`).
2. Stage A. Append `name = 0xADDR; // size:0xN` to `configs/USA/sym.main.txt`
   and delete the line from `linkers/USA/undefined_syms_manual.txt`.
3. Unify names. Replace the other spellings in `src/main` and the headers
   main includes (here `D_8009D20C` in 8 units and `battle_runtime.h`).
   Overlay sources may keep address-form names; they resolve by address.
4. `make split && make build && make check`. Then check that
   `linkers/USA/undefined_addr_aliases.main.txt` did not gain the old name; if
   it did, a reference was missed (the first try of the pilot did exactly
   that: 18 more pinned words until the battle units were renamed).
5. If a rename changes code (lev > 0), revert it and add a relative alias
   `old = name;` to `undefined_syms_manual.txt` instead.
6. Stage B, when the group allows it (object runs only). Define the variable
   in the owner (`static FieldSfxQueueEntry g_FieldSfxQueue[28];`), drop the
   header `extern` if nothing else uses it, and split the blob in
   `configs/USA/main.yaml` at the variable's file offsets
   (`vram - 0x80010000 + 0x800`) with a `.bss`/`.sbss`/`.data` row for the
   unit (`linker_section_order: .data`) and a new blob row after it. Check
   that the unit's section has exactly the gap's size (`objdump -h`).
7. `make split build check`, all overlays, the checks and `make report`;
   `make shift-test` shows the symbol gone from the pinned lists.

What the pilot learned: `g_FieldSfxQueueCount` could not be owned. Its unit
runs cc1 at `-G0` (`MASPSX_FORCE_G0`), so `= 0` puts it in `.data` at the
unit's `.data` position and everything after it moved by 4 bytes. It stays a
blob label until the unit runs cc1 with `-G8` or the forced-G0 path learns
`.sdata`.

## 5. Work list

### Order

Stage A is independent of code generation and is done by region packages
whose symbol sets are disjoint, so agents can run in parallel; each package
owns its symbols' lines in `sym.main.txt` (append in one block per package to
keep rebases trivial) and every C rename those symbols need.

| wave | package | symbols | words + gp | risk | notes |
| --- | --- | ---: | ---: | --- | --- |
| 0 | layout: engine boundary to 0x800C20C8, COMMON catch-all, `_end`/BSS-start symbols | 1 | 4 | low | done: engine at 0x800C20C8; `tools/scripts/ld_postpass.py` (run by `make split`) puts main's BSS in `.main`, adds `*(.sdata)` at `__bss_start`, `*(.sbss) *(.scommon)` at the small pool end 0x8009D310 (the gp window: new small data at the end of main overflows `R_MIPS_GPREL16`) and `*(COMMON)` before `_end`; `snmain` uses `__bss_start`/`_end`; a new global and function link and main grows by their size (`test_ld_postpass.py`) |
| 1 | functions named elsewhere, shadowing pins, field engine labels | 24 | 131 | low | names only |
| 2 | `.bss` object run | 60 | 1016 | low | stage A done (2a): 58 blob labels in `sym.main.txt`, 18 address-form C names unified, no alias; pinned words 6253 to 5237; stage B open |
| 2 | `.sbss` object run + `.sdata` | 108 | 1183 | medium | 99 COMMON pins; names used by many menu units. Stage A done: 107 pins removed (blob labels; 5 second views are relative aliases); left `g_ScriptCameraBytes` (label shared with the BSS start in `snmain`, wave 0) |
| 2 | large COMMON pool | 104 | 1984 | medium | stage A done: 103 pins are blob labels (pinned words 6253 to 4293), 28 C renames, 40 relative aliases for differently typed record views; `D_800C20C8` left to the engine boundary |
| 3 | small COMMON pool | 67 | 1870 | high | stage A done (3b): 71 pins are blob labels (pinned words 1814 to 392, gp-relative to pinned 298 to 6); 18 address-form names unified in C (`D_8009D1A0`, `D_8009D278`, `D_8009D26C` among them), 29 differently typed views are relative aliases (incl. `g_GameStateBlock`, no longer an `__asm__` alias); `g_PlayerEntity` stays `field/actor_collision`'s `D_8009D254` with its two aliases |
| 3 | `.data` and `.rodata` in blobs (game, PsyQ SPU/GPU/pad/DS) | 96 | 1114 | medium | stage A done: 93 blob labels for 94 pins, pinned words 6253 to 5151; 7 type/record views kept as relative aliases (`g_PmTables`, `g_GpuDmaState`, `g_GpuDmaRegisterWindow`, `D_80095674`, `D_8009B46C`, `D_8009B784`, `D_8009B788`); the three `engine_800C1CA0` labels belong to the large COMMON pool |
| 3 | `psyq/libcd` state block in `bios_4.c` | 40 | 388 | high | done (3c): `bios_4.c` defines the BIOS state, the hardware register pointers and the event bytes as separate globals (the two structs are gone) and the three command tables as `g_CdCommandTables`; 26 address-form and 19 pinned names unified in 15 LIBCD/LIBDS units, no alias left; pinned words 1814 to 1426 |
| 4 | literals: 15 data words, 8 `lui`, 13 casts, header PC | | 23 | medium | |
| 5 | stage B by unit, object runs first (`.bss`, then `.data`, then `.sbss` once MASPSX allows) | | | per unit | uses the per-module table below |
| 6 | overlays link against built main, `PE.IMG` repack | | | high | needed before a shifted build can boot |

Wave 0b status (3.4): done, byte-identical. `dtail_0D0860` is split into `engine/engine_data_800E0060`, `engine/engine_bss_800E21A8` and `engine/boot_work_area` (all in `.field_engine`, moving with main) and a new `overlay_windows` segment anchored at 0x8010BCF8 (VMA and file offset fixed; `ld_postpass.py` asserts that main and the engine end before it and defines `overlay_windows_HEADROOM`). The "free gap" is not free: 0x800F34F8..0x8010BCF8 is `g_BootWorkArea` (was `D_800F34F8`), the 0x18800-byte area `Boot_InitMemoryLayout` carves at 0x1800/0x6000/0x14000, so the retail headroom is 0 and main can grow only by shrinking it (`test_ld_postpass.py` shows growth past the window failing and growth into 0x100 bytes of room moving the engine and work area as a unit).

Floating window block status: done, byte-identical. The `overlay_windows` segment follows the boot work area; its six windows are offsets in `linkers/USA/overlay_windows.txt`, overlays link at them from the built main, the header's text size and `$sp` come from the link and the limit is the initial stack (0x1FF0 bytes of room). `make shift-full-test` relinks main and all 190 overlays shifted by a pad and explains every differing word (section 6).

Wave 6 status (`PE.IMG` repack): `make pe-img-repack`/`pe-img-check` rebuild `PE.IMG` in place from the built overlays, byte-identical at retail; the sector tables and room headers that record overlay sizes are checked (3.7).

Wave 1 status: done, byte-identical; 18 function pins, `g_DsReadStatusBlock`, 3 `fieldfx/burst` labels and 4 `FieldFx_*` symbolic aliases gone (pinned words 6253 to 6140); `D_800C20C8` (`_end`) is left to wave 0, `func_800119CC` (rodata) to the data package.

Within a package, work from the least shared symbols to the most shared. A
symbol referenced by several modules belongs to the package of its group,
not to a module; the module table below is the order for stage B and for
assigning agents, and a module's stage A is complete when its package is.

### Per module

Pinned symbols referenced (own = referenced by no other main unit), words
(address and gp-relative relocations to pinned symbols), the groups they lie
in, the most used, and the risk (high: many shared or aliased names, or more
than 300 words; aliased = addresses with more than one name). Ordered low to
high risk, then by size. `script/sfx_queue` (pilot) is done.

| # | module | pins | words | groups | most used | risk |
| --- | --- | --- | ---: | --- | --- | --- |
| 1 | `asm:dtail_gp_post_dsfile` | 1 (1 own) | 1 | func 1 | `func_80090F68` | low (1 aliased) |
| 2 | `psyq/libgpu/sys_3` | 1 (1 own) | 2 | data 1 | `g_GpuActiveDispEnv` | low |
| 3 | `psyq/libcd/c_003` | 1 (0 own) | 2 | data 1 | `D_8009AFD8` | low |
| 4 | `psyq/libspu/s_snc` | 1 (0 own) | 2 | data 1 | `_spu_RXX` | low |
| 5 | `psyq/libspu/s_gvex` | 1 (0 own) | 2 | data 1 | `_spu_RXX` | low |
| 6 | `psyq/libspu/s_srmd` | 1 (0 own) | 2 | data 1 | `_spu_RXX` | low |
| 7 | `psyq/libspu/s_r` | 1 (0 own) | 2 | data 1 | `_spu_transferCallback` | low |
| 8 | `psyq/libspu/s_w` | 1 (0 own) | 2 | data 1 | `_spu_transferCallback` | low |
| 9 | `draw/color_quad` | 1 (0 own) | 2 | sbss 1 | `D_8009D10C` | low (1 aliased) |
| 10 | `psyq/libcd/c_005` | 1 (0 own) | 2 | common 1 | `g_StrDataReadyCallback` | low |
| 11 | `psyq/libcd/c_008` | 1 (0 own) | 2 | common 1 | `StRingAddr` | low (1 aliased) |
| 12 | `menu/item_detail` | 1 (0 own) | 3 | sbss 1 | `g_MenuItemPrimListHandles` | low |
| 13 | `psyq/libds/dssys_3` | 1 (0 own) | 4 | data 1 | `CD_debug` | low (1 aliased) |
| 14 | `psyq/libspu/s_svva` | 1 (0 own) | 4 | data 1 | `_spu_RXX` | low |
| 15 | `psyq/libspu/s_stc` | 1 (0 own) | 4 | data 1 | `_spu_transferCallback` | low |
| 16 | `psyq/libspu/s_sic` | 1 (0 own) | 4 | data 1 | `_spu_IRQCallback` | low |
| 17 | `psyq/libds/dsready` | 1 (0 own) | 4 | data 1 | `g_DsReadBusy` | low |
| 18 | `battle/enemy_slots` | 1 (0 own) | 4 | scommon 1 | `D_8009D2A0` | low |
| 19 | `psyq/libds/dsread2` | 1 (1 own) | 4 | common 1 | `g_DsStreamNoLocFlag` | low |
| 20 | `psyq/libcd/c_011` | 1 (0 own) | 4 | common 1 | `g_CdStreamEndState` | low |
| 21 | `psyq/libetc/vmode` | 1 (1 own) | 6 | data 1 | `g_VideoMode` | low |
| 22 | `battle/player_turn` | 1 (0 own) | 7 | scommon 1 | `D_8009D1D4` | low (1 aliased) |
| 23 | `psyq/libspu/s_si` | 1 (0 own) | 8 | data 1 | `_spu_RXX` | low |
| 24 | `psyq/libds/dsread_2` | 1 (1 own) | 8 | data 1 | `g_CdReadCompleteCallback` | low |
| 25 | `psyq/libetc/vsync` | 1 (0 own) | 10 | data 1 | `g_VSyncCount` | low |
| 26 | `psyq/libapi/pad` | 1 (1 own) | 10 | data 1 | `g_InitPadFlag` | low |
| 27 | `psyq/libds/dstype` | 1 (1 own) | 10 | common 1 | `g_DsDiskType` | low |
| 28 | `psyq/libds/dsfile` | 1 (0 own) | 12 | data 1 | `D_8009AFC0` | low (1 aliased) |
| 29 | `fieldfx/emitters` | 1 (0 own) | 12 | scommon 1 | `g_PlayerEntity` | low (1 aliased) |
| 30 | `render/frame_update` | 1 (0 own) | 16 | sdata 1 | `g_ActiveDrawSlot` | low (1 aliased) |
| 31 | `psyq/libds/dsfile_2` | 1 (0 own) | 20 | data 1 | `D_8009AFC0` | low (1 aliased) |
| 32 | `psyq/libspu/s_sca` | 1 (0 own) | 28 | data 1 | `_spu_RXX` | low |
| 33 | `menu/save_slot` | 1 (0 own) | 30 | common 1 | `g_TextboxEntries` | low |
| 34 | `cdrom/track_read` | 1 (0 own) | 34 | common 1 | `g_GameState` | low |
| 35 | `psyq/libspu/s_sra` | 1 (0 own) | 64 | data 1 | `_spu_RXX` | low |
| 36 | `draw/textbox` | 1 (0 own) | 72 | common 1 | `g_TextboxEntries` | low |
| 37 | `menu/equip_screen` | 2 (0 own) | 3 | sbss 2 | `g_InvAmmoSpendActiveList`, `g_MenuEquipSwapSource` | low |
| 38 | `psyq/libgpu/sys_7` | 2 (0 own) | 4 | data 2 | `g_GpuGp0Ptr`, `g_GpuGp1Ptr` | low |
| 39 | `psyq/libds/dssys_1_2` | 2 (0 own) | 4 | data 2 | `g_DsReadSysEnabled`, `g_CdSeekState` | low |
| 40 | `psyq/libcd/c_010` | 2 (1 own) | 4 | common 2 | `g_CdStreamEndState`, `g_CdStreamMask` | low |
| 41 | `psyq/libetc/intr_2` | 2 (0 own) | 6 | data 2 | `g_EventCallbackTable`, `g_IntrMaskRegPtr` | low |
| 42 | `psyq/libspu/s_stsa` | 2 (0 own) | 6 | data 2 | `g_SpuTransferAddr`, `_spu_mem_mode` | low |
| 43 | `psyq/libspu/s_m_util` | 2 (0 own) | 6 | data 2 | `_spu_memList`, `_spu_mem_mode` | low |
| 44 | `psyq/libds/dssys_1_6` | 2 (0 own) | 6 | data 1, bss 1 | `g_DsReadyCallback`, `g_CdSeekState` | low |
| 45 | `boot/frame_loop` | 2 (0 own) | 6 | scommon 1, common 1 | `g_GameState`, `g_GameStateFlags` | low (1 aliased) |
| 46 | `psyq/libcd/c_007` | 2 (0 own) | 6 | common 2 | `StRingAddr`, `StRingSize` | low (1 aliased) |
| 47 | `psyq/libsn/snmain` | 2 (2 own) | 8 | rodata 1, engine 1 | `jtbl_80010000`, `D_800C20C8` | low (2 aliased) |
| 48 | `psyq/libds/dssys_1_8` | 2 (0 own) | 8 | data 2 | `g_DsReadSysEnabled`, `g_DsReadStatusBlock` | low |
| 49 | `battle/enemy_movement` | 2 (0 own) | 8 | scommon 2 | `D_8009D2A0`, `g_ActiveActor` | low (1 aliased) |
| 50 | `fieldfx/geometry` | 2 (2 own) | 8 | common 2 | `D_800B0E18`, `D_800B0E1C` | low (2 aliased) |
| 51 | `menu/equip_select` | 2 (0 own) | 9 | sbss 2 | `g_InvAmmoSpendActiveList`, `g_MenuSpendArrowDirection` | low |
| 52 | `fieldfx/sprite_effects` | 2 (0 own) | 12 | data 1, scommon 1 | `g_RoomFloorY`, `g_PlayerEntity` | low (1 aliased) |
| 53 | `psyq/libetc/intr_vb` | 2 (1 own) | 12 | data 2 | `g_IntrVSyncCallbackTable`, `g_VSyncCount` | low |
| 54 | `entity/allocation` | 2 (0 own) | 12 | scommon 1, common 1 | `g_PlayerEntity`, `g_RenderScratchBufferBase` | low (1 aliased) |
| 55 | `psyq/libpad/waitrc2` | 2 (2 own) | 14 | common 2 | `g_TimerTimeoutStart`, `g_TimerTimeoutLimit` | low |
| 56 | `psyq/libetc/intr` | 2 (0 own) | 18 | data 2 | `g_EventCallbackTable`, `g_IntrMaskRegPtr` | low |
| 57 | `psyq/libetc/intr_dma` | 2 (2 own) | 20 | data 2 | `g_IntrDmaDispatchPtr`, `g_IntrDmaHandlerTable` | low |
| 58 | `psyq/libgpu/sys` | 3 (3 own) | 6 | data 3 | `g_GpuCallbacks`, `g_GpuDebugPrintf`, `g_GraphDebug` | low |
| 59 | `psyq/libpad/padseqd` | 3 (1 own) | 6 | data 3 | `g_MemCardStateDispatchFn`, `g_MemCardIsTransferActiveFn`, `g_MemCardResponseHandler` | low |
| 60 | `time/counters` | 3 (2 own) | 6 | common 2, scommon 1 | `g_GameStateFlags`, `g_GameTimeTable`, `g_PlayTimeSeconds` | low (1 aliased) |
| 61 | `psyq/libcd/c_002` | 3 (0 own) | 6 | common 3 | `g_CdStreamDataReadyFlag`, `g_CdStreamRingReadSlot`, `StRingSize` | low |
| 62 | `psyq/libpad/padcmd` | 3 (2 own) | 8 | func 2, data 1 | `g_MemCardIsTransferActiveFn`, `func_80083C20`, `func_80083C3C` | low (2 aliased) |
| 63 | `psyq/libspu/s_q` | 3 (0 own) | 8 | data 3 | `_spu_EVdma`, `_spu_transferCallback`, `_spu_IRQCallback` | low |
| 64 | `psyq/libcd/c_009` | 3 (0 own) | 10 | common 3 | `StRingAddr`, `g_CdStreamEndState`, `StRingSize` | low (1 aliased) |
| 65 | `psyq/libds/dsready_2` | 3 (2 own) | 11 | func 2, data 1 | `g_DsReadBusy`, `func_8008227C`, `func_8007FC44` | low (2 aliased) |
| 66 | `psyq/libds/dssys_2_2` | 3 (0 own) | 12 | bss 3 | `g_CdDsReadQueue`, `g_CdPendingReadCount`, `g_DsReadCallbackCursor` | low |
| 67 | `psyq/libspu/s_crwa` | 3 (0 own) | 14 | data 3 | `_spu_transferCallback`, `_spu_mem_mode`, `g_SpuReverbWorkAreaTable` | low |
| 68 | `psyq/libpad/padentry` | 3 (0 own) | 18 | data 3 | `g_MemCardObjLookupFn`, `g_MemCardObjArray`, `g_MemCardCommandByte` | low |
| 69 | `psyq/libspu/spu_3` | 3 (0 own) | 26 | data 3 | `_spu_mem_mode`, `_spu_RXX`, `_spu_sys_pcr` | low |
| 70 | `script/value_ops` | 3 (0 own) | 62 | scommon 2, common 1 | `g_CurrentEntity`, `g_PlayerEntity`, `g_RenderStateFlags` | low (1 aliased) |
| 71 | `fieldfx/burst` | 3 (3 own) | 82 | engine 3 | `g_PointSpriteStreakSprite`, `g_PointSpriteState0`, `g_PointSpriteState1` | low |
| 72 | `psyq/libspu/s_m_init` | 4 (2 own) | 8 | data 4 | `_spu_mem_mode`, `_spu_AllocBlockNum`, `_spu_AllocLastNum` | low |
| 73 | `psyq/libds/dssys_1_10` | 4 (0 own) | 8 | bss 3, data 1 | `g_DsReadStatusBlock`, `g_DsPollCallback`, `g_DsSyncCallback` | low |
| 74 | `psyq/libcd/c_004` | 4 (1 own) | 10 | common 4 | `g_CdStreamRingReadSlot`, `g_StrDataReadyCallback`, `g_CdStreamDataReadyFlag` | low (1 aliased) |
| 75 | `psyq/libspu/s_srmt` | 4 (1 own) | 14 | data 4 | `_spu_RXX`, `_spu_rev_offsetaddr`, `g_SpuReverbWorkAreaTable` | low |
| 76 | `menu/skills` | 4 (1 own) | 14 | sbss 3, common 1 | `g_MenuInventoryViewMode`, `g_DiscChangeFlags`, `g_MenuEquipSwapSource` | low (1 aliased) |
| 77 | `boot/main` | 4 (0 own) | 14 | scommon 2, common 2 | `g_SceneDispatchToken`, `g_SceneDispatchCur`, `g_PlayTimeFrameCounter` | low |
| 78 | `psyq/libspu/s_sr` | 4 (0 own) | 18 | data 4 | `_spu_rev_flag`, `_spu_RXX`, `_spu_rev_reserve_wa` | low |
| 79 | `psyq/libgpu/sys_10` | 4 (0 own) | 40 | data 4 | `g_GpuDmaChcrPtr`, `g_GpuGp1Ptr`, `g_GpuDmaTimeoutDeadline` | low |
| 80 | `render/object` | 4 (2 own) | 66 | common 2, data 1, sdata 1 | `g_IdentityMatrixSource`, `g_ActiveDrawSlot`, `g_RenderClutLookupTable` | low (1 aliased) |
| 81 | `psyq/libspu/s_ini` | 5 (1 own) | 10 | data 5 | `_spu_EVdma`, `_spu_rev_flag`, `_spu_rev_reserve_wa` | low |
| 82 | `psyq/libds/dssys_2` | 5 (0 own) | 14 | bss 5 | `g_CdDsReadQueue`, `g_CdDsReadQueueState`, `g_CdDsReadIndex` | low |
| 83 | `psyq/libgpu/sys_8` | 5 (0 own) | 20 | data 5 | `g_GpuGp1Ptr`, `g_GpuDmaChcrPtr`, `g_GpuDmaControlRegPtr` | low |
| 84 | `psyq/libpad/padif` | 5 (2 own) | 24 | data 5 | `g_MemCardStateDispatchFn`, `g_MemCardDispatchResult`, `g_MemCardWriteBlockCount` | low |
| 85 | `psyq/libds/dssys_1_4` | 5 (1 own) | 34 | data 4, bss 1 | `g_CdRomCmdTimeout`, `g_CdRomCmdLongTimeoutTable`, `g_DsPollCallback` | low |
| 86 | `psyq/libds/dssys_2_3` | 6 (1 own) | 22 | bss 5, common 1 | `g_DsReadCallbackCursor`, `g_CdPendingReadCount`, `g_CdDsReadQueue` | low |
| 87 | `psyq/libgpu/sys_6` | 6 (2 own) | 42 | data 5, bss 1 | `g_GpuGp0Ptr`, `g_GpuGp1Ptr`, `g_GpuDmaMadrPtr` | low |
| 88 | `psyq/libspu/spu_2` | 7 (3 own) | 56 | data 7 | `_spu_RXX`, `g_SpuDmaDirection`, `g_SpuTransferAddr` | low |
| 89 | `cdrom/scene_assets` | 8 (3 own) | 38 | common 6, data 2 | `g_GameState`, `g_Base32CharTable`, `g_DiscChangeFlags` | low |
| 90 | `psyq/libpad/padmain` | 8 (3 own) | 44 | data 8 | `g_MemCardSioRegs`, `g_MemCardPort1Present`, `g_MemCardPort2Present` | low |
| 91 | `render/font` | 1 (1 own) | 110 | data 1 | `g_FontSelectionState` | medium |
| 92 | `menu/main_update` | 2 (0 own) | 16 | sdata 1, scommon 1 | `g_ActiveDrawSlotRecord`, `D_8009D1D4` | medium (2 aliased) |
| 93 | `battle/context` | 3 (0 own) | 12 | scommon 2, data 1 | `g_ActiveActor`, `g_PlayerEntity`, `g_RoomFloorY` | medium (2 aliased) |
| 94 | `asset/unload_table` | 3 (0 own) | 16 | data 1, scommon 1, common 1 | `g_PmCmdHandlerTable`, `g_GameStateFlags`, `g_GameState` | medium (2 aliased) |
| 95 | `battle/commands` | 3 (0 own) | 18 | common 2, scommon 1 | `g_PlayerEntity`, `g_AyaEquippedWeaponSlot`, `g_AyaParasiteSpellFlags` | medium (2 aliased) |
| 96 | `asm:dtail_gp_pre_s016` | 4 (4 own) | 4 | func 4 | `func_80017928`, `func_80017948`, `func_80017968` | medium (4 aliased) |
| 97 | `draw/frame` | 4 (0 own) | 15 | common 3, sdata 1 | `g_ActiveDrawSlot`, `g_GameState`, `g_RenderDrawEnvArray` | medium (1 aliased) |
| 98 | `battle/phase` | 5 (0 own) | 22 | scommon 3, data 1, sbss 1 | `D_8009D1D4`, `g_PmSlotTable`, `g_BattleEffectPmSlot` | medium (1 aliased) |
| 99 | `menu/inventory` | 5 (1 own) | 28 | sbss 5 | `g_MenuActiveWidget`, `g_MenuSelectedItemData`, `g_MenuEquipSwapSource` | medium (1 aliased) |
| 100 | `asset/tim_textures` | 5 (1 own) | 32 | data 3, scommon 1, common 1 | `g_GameStateFlags`, `g_PmCmdHandlerTable`, `g_PmTables` | medium (3 aliased) |
| 101 | `item/swap` | 5 (1 own) | 32 | common 3, sbss 2 | `g_EquippedWeaponSlot`, `g_EquippedArmorSlot`, `g_EquipItemDataTable` | medium (2 aliased) |
| 102 | `pm/process` | 5 (1 own) | 122 | data 3, scommon 1, common 1 | `g_PmSlotTable`, `g_PmSlotTable2`, `g_GameState` | medium (2 aliased) |
| 103 | `psyq/libpad/padportd` | 6 (0 own) | 14 | data 6 | `g_MemCardServiceReady`, `g_MemCardObjResetFn`, `g_MemCardObjLookupFn` | medium |
| 104 | `aya/stats` | 6 (3 own) | 17 | scommon 2, common 2, sbss 1, bss 1 | `g_AkaoBgmHandle`, `g_GameStateFlags`, `g_PlayerEntity` | medium (2 aliased) |
| 105 | `psyq/libds/dssys_1_5` | 6 (0 own) | 34 | data 3, bss 3 | `g_DsSyncCallback`, `g_DsDispatchCallback`, `g_DsReadSysEnabled` | medium |
| 106 | `entity/combat` | 6 (4 own) | 38 | scommon 6 | `g_PlayerEntity`, `g_ActiveActor`, `g_BattleHitActionMode` | medium (2 aliased) |
| 107 | `battle/encounter` | 6 (2 own) | 42 | scommon 3, func 1, sbss 1, common 1 | `g_BattleEndTimer`, `g_GameState`, `g_BattleEquipStateBlock` | medium (1 aliased) |
| 108 | `battle/enemy_actions` | 6 (2 own) | 58 | scommon 6 | `g_ActiveActor`, `g_ActorEffectFlag100Timer`, `g_FieldMoveLock` | medium (3 aliased) |
| 109 | `script/node_actions` | 6 (0 own) | 121 | scommon 4, sbss 2 | `g_CurrentEntity`, `g_ScriptNodePool`, `g_SceneDataTable0` | medium (2 aliased) |
| 110 | `draw/pipeline` | 7 (1 own) | 14 | scommon 3, common 2, data 1, sdata 1 | `g_GameState`, `g_RoomFloorY`, `g_GameStateFlags` | medium (1 aliased) |
| 111 | `psyq/libds/dssys_1` | 7 (0 own) | 16 | data 4, bss 3 | `g_DsReadyCallback`, `g_CdSyncCallback`, `g_CdReadyCallback` | medium (2 aliased) |
| 112 | `script/node_pool` | 7 (2 own) | 21 | sbss 3, scommon 2, bss 1, common 1 | `g_ScriptNodeFreeListHead`, `g_SceneDataTable1`, `g_ScriptNodeSeqCounter` | medium (1 aliased) |
| 113 | `psyq/libspu/spu` | 7 (0 own) | 60 | data 7 | `_spu_RXX`, `_spu_mem_mode`, `g_SpuTransferAddr` | medium |
| 114 | `entity/pool` | 8 (0 own) | 14 | scommon 5, common 3 | `g_PlayerEntity`, `g_EntityFreePoolCount`, `g_EntityFreeListHead` | medium (2 aliased) |
| 115 | `psyq/libgpu/sys_9` | 9 (0 own) | 42 | data 9 | `g_GpuGp1Ptr`, `g_GpuDmaQueueTail`, `g_GpuGp0Ptr` | medium |
| 116 | `menu/item_actions` | 9 (4 own) | 66 | sbss 6, common 2, bss 1 | `g_BonusPointSpendStat`, `g_BonusPointSpendWorkingValue`, `g_BonusPointStatDeltas` | medium (1 aliased) |
| 117 | `render/mesh_list` | 9 (3 own) | 92 | common 6, data 1, sdata 1, scommon 1 | `g_GeomState`, `g_RenderStateFlags`, `g_ActiveDrawSlot` | medium (2 aliased) |
| 118 | `draw/wipe` | 9 (3 own) | 116 | sbss 7, bss 1, common 1 | `g_TextCursorStackTop`, `g_DrawVertexWritePtr`, `g_TextCursorX` | medium (1 aliased) |
| 119 | `script/flow_opcodes` | 9 (1 own) | 131 | scommon 6, sbss 2, common 1 | `g_CurrentEntity`, `g_ScriptNodePool`, `g_SceneDataTable0` | medium (5 aliased) |
| 120 | `menu/save_screens` | 10 (3 own) | 39 | sbss 6, bss 2, common 2 | `D_800A19C0`, `g_MenuRenameTargetRecord`, `g_SaveSelectedSlot` | medium (1 aliased) |
| 121 | `menu/lists` | 11 (0 own) | 29 | sbss 7, common 3, bss 1 | `g_MenuActiveWidget`, `g_BonusPointStatDeltas`, `g_SaveSelectedSlot` | medium (2 aliased) |
| 122 | `field/actor_collision` | 11 (5 own) | 35 | scommon 6, sbss 5 | `g_FloorBlockedEdgePointA`, `g_CollisionPlaneTable`, `g_CollisionDb` | medium (5 aliased) |
| 123 | `render/camera` | 11 (5 own) | 176 | common 10, scommon 1 | `g_GeomState`, `g_RenderStateFlags`, `g_GeomVramPacketDst` | medium (1 aliased) |
| 124 | `scene/room_load` | 12 (4 own) | 128 | common 7, scommon 4, data 1 | `g_GameState`, `g_PlayerEntity`, `g_GameStateFlags` | medium (4 aliased) |
| 125 | `boot/init` | 13 (6 own) | 95 | scommon 7, common 5, sdata 1 | `g_RenderDrawEnvArray`, `g_GameStateFlags`, `g_RenderDispEnvArray` | medium (2 aliased) |
| 126 | `menu/main_menu` | 15 (7 own) | 85 | sbss 9, bss 4, common 2 | `g_MenuPendingTotalExp`, `g_MenuLevelDisplayValue`, `g_BonusPointStatDeltas` | medium |
| 127 | `psyq/libds/dssys_1_3` | 16 (6 own) | 34 | data 12, bss 4 | `g_CdRomCmdTimeout`, `g_CdSyncCallback`, `g_CdReadyCallback` | medium (2 aliased) |
| 128 | `draw/text` | 16 (5 own) | 83 | sbss 12, bss 2, sdata 1, common 1 | `g_TextCursorStackBottom`, `g_TextCursorStackTop`, `D_8009D10C` | medium (2 aliased) |
| 129 | `item/armor_capacity` | 16 (10 own) | 103 | sbss 9, data 3, common 3, bss 1 | `g_InvStorageSelectionBits`, `g_InvSortListBase`, `g_InvSortListCount` | medium (2 aliased) |
| 130 | `akao/spu_transfer` | 17 (2 own) | 78 | scommon 11, common 5, sdata 1 | `g_AkaoSpuCommonAttr`, `g_SpuTransferStatus`, `g_AkaoTimerEventDesc` | medium |
| 131 | `menu/input_queue` | 18 (8 own) | 67 | sbss 11, common 4, sdata 1, scommon 1, bss 1 | `g_MenuEventQueueFreeList`, `g_MenuEventQueueHead`, `g_MenuEventQueueTail` | medium (1 aliased) |
| 132 | `menu/status_panels` | 18 (7 own) | 71 | sbss 9, common 5, bss 4 | `g_MenuSaveBgFadeState`, `g_MenuSaveBgFadeHeight`, `g_MenuSaveBgFadeStep` | medium |
| 133 | `menu/widget` | 18 (8 own) | 202 | sbss 12, bss 5, sdata 1 | `g_MenuWidgetColumnLayoutTable`, `g_MenuWidgetActiveListHead`, `g_TextCursorStackPtr` | medium (2 aliased) |
| 134 | `pm/disc_tables` | 19 (5 own) | 74 | common 14, data 3, sdata 1, scommon 1 | `g_GameState`, `g_DrawEnabled`, `g_SeqElapsed` | medium (1 aliased) |
| 135 | `akao/sequencer` | 22 (5 own) | 188 | scommon 9, common 9, data 4 | `g_AkaoCurTrack`, `g_AkaoVoiceUpdateFlags`, `g_AkaoInstrumentTable` | medium |
| 136 | `psyq/libcd/sys` | 4 (1 own) | 8 | data 4 | `D_8009AFC4`, `g_CdLastPos`, `g_CdMode` | high (1 aliased) |
| 137 | `psyq/libcd/bios_6` | 4 (0 own) | 8 | data 4 | `g_CdLastCom`, `D_8009AFDC`, `D_8009B05C` | high (2 aliased) |
| 138 | `psyq/libcd/bios_2` | 7 (0 own) | 32 | data 7 | `D_8009B27C`, `D_8009AFB4`, `D_8009AFB8` | high (5 aliased) |
| 139 | `psyq/libcd/sys_2` | 7 (3 own) | 50 | data 6, rodata 1 | `D_8009AFB4`, `D_8009AFC4`, `func_800119CC` | high (5 aliased) |
| 140 | `psyq/libcd/bios_4` | 9 (1 own) | 26 | data 9 | `g_CdRegIndexBase`, `g_CdRegDataWrite`, `g_CdRegResponse` | high (6 aliased) |
| 141 | `psyq/libcd/bios_5` | 12 (1 own) | 36 | data 12 | `D_8009B288`, `D_8009B27C`, `D_8009AFC4` | high (6 aliased) |
| 142 | `entity/frame_update` | 12 (5 own) | 44 | scommon 6, func 4, common 2 | `g_GameState`, `g_PlayerEntity`, `g_EntityFreeListHead` | high (7 aliased) |
| 143 | `psyq/libcd/bios` | 13 (3 own) | 62 | data 13 | `D_8009B294`, `D_8009B27C`, `D_8009AFC4` | high (8 aliased) |
| 144 | `psyq/libcd/bios_7` | 14 (6 own) | 46 | data 14 | `D_8009B27C`, `g_CdRegIndexBase`, `D_8009B28C` | high (8 aliased) |
| 145 | `item/actions` | 14 (2 own) | 133 | common 7, bss 4, sbss 3 | `g_InvCompareSlotLeft`, `g_InvCompareSlotRight`, `g_InvStorageSelectionBits` | high (4 aliased) |
| 146 | `script/actor_opcodes` | 15 (4 own) | 192 | scommon 8, data 3, common 2, func 1, sbss 1 | `g_CurrentEntity`, `g_GameStateFlags`, `g_ScriptNodePool` | high (6 aliased) |
| 147 | `battle/setup` | 16 (2 own) | 70 | scommon 12, sbss 3, bss 1 | `g_ActiveActor`, `g_BattleTargetList`, `g_GameStateFlags` | high (9 aliased) |
| 148 | `asset/game_state` | 17 (1 own) | 110 | common 14, scommon 2, sdata 1 | `g_GameState`, `g_PeImageMountFlags`, `g_ActiveDrawSlot` | high (2 aliased) |
| 149 | `battle/status_hud` | 17 (9 own) | 243 | scommon 7, sbss 3, bss 3, sdata 2, common 2 | `g_ActiveDrawSlot`, `g_MenuActiveWindowXOffset`, `g_OtBufferTable` | high (6 aliased) |
| 150 | `battle/turn` | 18 (4 own) | 183 | scommon 11, sbss 5, bss 1, common 1 | `g_BattleTargetList`, `D_8009D2D8`, `g_AkaoBgmHandle` | high (7 aliased) |
| 151 | `akao/voice_driver` | 18 (1 own) | 518 | common 13, scommon 4, sdata 1 | `g_AkaoCmd`, `g_AkaoCurTrack`, `g_AkaoVoiceUpdateFlags` | high |
| 152 | `psyq/libcd/bios_3` | 19 (3 own) | 80 | data 19 | `g_CdRegResponse`, `g_CdRegIndexBase`, `D_8009B294` | high (16 aliased) |
| 153 | `script/system_opcodes` | 20 (2 own) | 294 | scommon 11, common 8, sbss 1 | `g_CurrentEntity`, `g_FieldMoveLock`, `g_GameStateFlags` | high (6 aliased) |
| 154 | `item/active_list` | 21 (5 own) | 76 | common 10, sbss 7, bss 4 | `g_EquipItemDataTable`, `g_ItemDropRngXorKeyHigh`, `g_ItemDropRngXorKeyLow` | high (4 aliased) |
| 155 | `save/game_data` | 21 (6 own) | 134 | common 11, sbss 7, scommon 2, bss 1 | `g_SaveRuntimeState`, `g_SaveMetadataCursor`, `g_SaveMetadataWindowLength` | high (4 aliased) |
| 156 | `battle/attack` | 21 (14 own) | 204 | scommon 13, sbss 7, bss 1 | `g_ActiveActorRecord`, `g_BattleResultFlagsRecord`, `D_8009D25C` | high (8 aliased) |
| 157 | `akao/voice_bank` | 30 (9 own) | 429 | scommon 13, common 12, func 3, data 1, sdata 1 | `g_AkaoCurTrack`, `g_AkaoCmd`, `g_SpuActiveVoiceMask` | high (3 aliased) |
| 158 | `memcard/save_flow` | 37 (29 own) | 544 | bss 26, common 6, scommon 3, data 2 | `g_SaveIoCursor`, `g_MemCardPortStates`, `g_GameState` | high (1 aliased) |

## 6. What cannot shift: disc and external ABI

Since the floating window block (below), no program address of the build is
fixed except the load address. Everything else in this table is either
outside the program (BIOS, hardware, disc positions) or the prelinked
overlays on the retail disc.

| item | value | stated in | why fixed |
| --- | --- | --- | --- |
| PS-X EXE load address | `.main` at 0x80010000 | `configs/USA/main.yaml`, header `.text start` | BIOS loader contract |
| Initial stack pointer | 0x801FFFF0, `PSX_EXE_STACK_TOP` | `tools/scripts/ld_postpass.py`, header `$sp` field | BIOS convention; the startup then sets `$sp` to `_ramsize - 8` and reserves `_stacksize` (0x2000) below it |
| Entry point | `__SN_ENTRY_POINT` (retail 0x80072534) | header `Initial PC` | not fixed: written by the link |
| Image size | `PSX_EXE_TEXT_SIZE` = end of the window block - 0x80010000 (retail 0x1EE000) | header `.text size` | not fixed: computed by the link |
| Overlay window layout | 6 windows as offsets from `overlay_windows_VRAM` | `linkers/USA/overlay_windows.txt` | the 190 overlays in the retail `PE.IMG` are prelinked at these offsets from the retail base 0x8010BCF8; the build relinks them for any base (below) |
| Retail window base 0x8010BCF8 | `vram:` of the `overlay_windows` segment | `configs/USA/main.yaml` (disassembly only) | the loader window base the shipped overlays were linked for; the link does not use it |
| `PE.IMG` sector layout | sector numbers and sizes in main's tables (the special-scene table at 0x80094048, `g_SceneSectorDirectory`, `D_800930D8`) | data in main | disc positions, not program addresses; a shift does not change any overlay's size, so rebuilt overlays fit their sectors |
| Save files | card file: 0x100-byte header + 0x12E4-byte copy of `g_SaveRuntimeState` | `memcard/save_flow`, `save/game_data` | the layout is the ABI; no typed field holds a program address, the untyped `inventory` (0x1064) and `remainder` (0x20C) parts still need checking |
| Kernel and BIOS | 0x80000000 vectors, 0xA000xxxx, LIBCARD's kernel patch at 0xDFFC | PsyQ LIBAPI/LIBCARD | BIOS contract |
| Hardware and scratchpad | 0x1F80xxxx | GPU, SPU, CD, pad drivers | hardware |
| External cheat devices | e.g. JP debug flag 0x8009CD3C | none | not ours; a shifted build breaks GameShark codes, accepted |

### The floating window block

The `overlay_windows` segment (0x8010BCF8..0x801FE000 in retail: the
mdec_diag, menu_memcard, boot_display, fx_common_sector17, room/scene/field
effect and task-program windows and the runtime buffers inside them) follows
`g_BootWorkArea` (`vram_symbol: field_engine_VRAM_END`), in memory and in the
file. One table names the windows:

| window | offset | retail |
| --- | ---: | --- |
| `OVERLAY_WINDOW_MDEC_DIAG` | 0x0 | 0x8010BCF8 |
| `OVERLAY_WINDOW_MENU_MEMCARD` | 0x15008 | 0x80120D00 |
| `OVERLAY_WINDOW_BOOT_DISPLAY` | 0x172AC | 0x80122FA4 |
| `OVERLAY_WINDOW_FX_SECTOR17` | 0x64308 | 0x80170000 |
| `OVERLAY_WINDOW_ROOM` | 0x832F0 | 0x8018EFE8 (fx_common at +8) |
| `OVERLAY_WINDOW_TASK_PROGRAM` | 0xE1B00 | 0x801ED7F8 |

- Main: `main.ld` INCLUDEs the table (`ld_postpass.py`), so `main.elf`
  defines the window symbols. Main's own references into the block are
  labels of the `main/overlay_windows` blob (`D_8010BCF8`, `D_80120D00`,
  `g_StrFileDirArea`, `g_PmTaskProgramArea`, `D_8010BD00`, `g_SceneLoadScratchArea`,
  the menu_memcard and boot entry points, the process slots
  `g_FieldFxTaskProgramCmd108`..`D_801F1F38`), so they move with the block.
- Overlays: `make overlay-split` rewrites each overlay's linker script so
  every section VMA is `(OVERLAY_WINDOW_x + offset)`
  (`tools/scripts/overlay_windows.py rebase-ld`); `overlay_link_symbols.py`
  writes the window values from the built `main.elf` and every ABI or
  own-window address as window plus offset; `linkers/USA/overlay_abi.txt`
  states its 465 entries the same way. The overlay configs keep retail
  `vram`s for the disassembly; `overlay_windows.py check` (and its test)
  holds them to the table.
- Header and limit: the text size is `PSX_EXE_TEXT_SIZE`, the initial `$sp`
  `PSX_EXE_STACK_TOP`; the link fails when the image would end above the
  initial stack pointer. `PSX_EXE_HEADROOM` is the room: 0x1FF0 bytes in the
  retail layout. Two softer limits: the startup's stack area starts at
  0x801FDFF8, 8 bytes below the retail image end, and the highest block
  address any object (main or overlay) refers to is 0x801F1F3A, so the
  referenced window memory can move 0xC0BE bytes before it reaches that stack
  area; and the BIOS loads the image while running on its own stack near the
  top of RAM, so a growth close to the full 0x1FF0 is untested on hardware.

Proof: `make shift-full-test` (`tools/scripts/shift_full_test.py`) relinks
main with a pad at the start of `.main` and all 190 overlays against the
shifted main, into a scratch directory, and compares every image with the
retail one. With pad 0x10: no fixed placement, the block and every window
symbol move by 0x10, all 190 base relinks are byte-identical to the built
overlays, every overlay section moves by 0x10, and every differing word is a
relocated address field moved by the pad (23818 in main plus the header's
text size; 87922 in the overlays); nothing else differs. The 10 relocated
words of main that did not move are main's remaining pins
(`jtbl_80010000`, `g_ScriptCameraBytes`), and the 10 literal words are the
register block and non-addresses of section 2. The same holds for pad
0x1FF0, and 0x1FF4 fails the link at the stack assertion.

What a shifted build still lacks to boot from a disc is the repack: the ISO
target reuses the retail `PE.IMG`, whose overlays are linked for the retail
base. Writing the rebuilt overlays into their sectors (same sizes) is wave 6.

`g_BootWorkArea` (0x800F34F8, 0x18800 bytes) sits before the block and moves
with the engine; the entity texture read at its +0x14000
(`Scene_LoadEntityTextures` case 4) can run past 0x8010BCF8 in retail (the
`D_800930D8` sector table allows reads of up to 0x20 sectors from index
0x16), so the first window buffer is probably time-shared with it; a RAM
trace would settle how much of the work area is really needed. Data in
`PE.IMG` (room directories, scripts, TIM, AKAO) holds no 32-bit pointer into
main; the room directory is relocated into `g_GameState` by offsets.

## 7. Pilot: `script/sfx_queue`

Three pinned symbols referenced, the smallest game unit with pins in three
different groups.

| symbol | group | result |
| --- | --- | --- |
| `g_FieldSfxQueue` (0x150 bytes) | `.bss` object run | stage B: `static` in `sfx_queue.c`, `.bss` row at 0x93980, blob split (`dtail_gp_post_sfx_queue`) |
| `g_FieldSfxQueueCount` | `.sdata` | stage A: blob label; stage B blocked by forced-G0 (section 4) |
| `g_FieldActorListHead` | small COMMON pool | stage A: blob label; `D_8009D20C` renamed in `battle/{attack,encounter,phase,setup,turn}`, `script/{actor_opcodes,flow_opcodes}`, `menu/main_update` and `battle_runtime.h` |

Commits on the branch:

1. `Link the field sfx queue state by name instead of absolute pins` (pilot,
   plus `gen_expected.py` counting in-image `.bss` and its tests).
2. `Derive $gp and the field engine address from the layout` (`_ramsize`,
   `_stacksize`, `ld_gp_expression`, `vram_symbol`).

Before and after (shift test, pad 0x10):

| measure | before | after |
| --- | ---: | ---: |
| fixed placements in the linker script | `.field_engine`, `_gp` | none |
| padded relink without rebasing | fails (overlap) | links |
| pinned address words | 6498 | 6443 |
| gp-relative words to pinned symbols | 1265 | 1243 |
| distinct pinned program symbols used | 503 | 499 |
| referenced assignments in `undefined_syms_manual.txt` | 483 | 480 |
| `script/sfx_queue` relocations to pinned symbols | 17 | 0 |

`make check`, all 190 overlays, `organization-check`, `source-policy-check`,
`debt-check` and `make report` pass. No function changed (lev 0 everywhere,
byte-identical image).

## 8. Decisions needed

- **MASPSX and `.comm` in `.sbss`.** MASPSX's default path turns a small
  `.comm` into a local `.sbss` label (only `.bss` labels get `.globl`), which
  is why 52 main units use `--use-comm-section` and keep real COMMON symbols.
  Owning the 102 `.sbss` object-run variables needs a global label there. If
  ASPSX did emit it global (to be checked against a PsyQ object with a small
  global `.comm`), this is a fidelity fix in MASPSX, not a crutch.
  *Checked 2026-10-13: ASPSX did not.* The raw PsyQ 4.0 `LIBGTE.LIB`
  `CLIP_INI.OBJ` declares `CLIPNearz`, `CLIPHdkx` and four more 4-byte
  globals as XBSS records (SN's common block, allocated by psylink), not as
  labels in the object's `.sbss`/`.bss`; the 4.0 signatures list XBSS for
  every small tentative global of LIBCD (`StRingSize`, `CChannel`, 20 in
  `CDROM.OBJ`), LIBDS, LIBETC and LIBSPU. No PsyQ library uses small data
  (all are -G0), so the `.sbss` case is seen in this game: the small COMMON
  pool after the `.sbss` object run is exactly psylink allocating small
  XBSS, and `StRingSize` (4 bytes, -G0 LIBCD) sits in the large pool.
  `--use-comm-section` is therefore the faithful mode and a global `.sbss`
  label would not be a fidelity fix. A `.sbss` object-run variable that
  several of our units reference is a file-static (`.lcomm`) of one retail
  object, i.e. TU-merge evidence, or stays a blob label.
- **COMMON pools.** Keep them as labelled blobs (shiftable, simplest), or
  model them as pool units in retail order. Recommendation: blobs now, pool
  units only if the PsyQ COMMON allocation order can be explained.
- **Manifest post-pass.** In-image BSS with a COMMON catch-all needs either a
  splat change or a small checked rewrite of `main.ld` after `make split`.
- **Relative linker aliases** (`old = new;`) as the fallback when unifying
  names would change code: allowed as migration state?

## 9. Remnants and catalogue (wave 4a)

### The last pins

| pin | words | now |
| --- | ---: | --- |
| `g_ScriptCameraBytes` (0x8009CDF8, `script/flow_opcodes`, a tentative global) | 6 gp-relative | blob label in `sym.main.txt`; it is the first word of the `.sbss` object run, which the startup now finds as `__bss_start`, so the label no longer stands for two things |
| `jtbl_80010000` (`psyq/libsn/snmain`, `__main` and `__do_global_dtors`) | 4 | `__sn_ctor_table` and `__sn_dtor_table`, defined by `ld_postpass.py` at the start of `.main`: SN's constructor and destructor tables, both empty in this game, which is why retail's linker put them at the image's first address |

After this, `shift_test.py` reports 0 pinned words and 0 gp-relative words to
pins, and the executable has no relocation against an absolute program
symbol.

### Literals that stay, with the proof

`shift_audit.py` (c) lists every address-shaped word without a relocation
with its reason (`KNOWN_LITERALS`):

- `Draw_InitGpuHwRegs` (`draw/gpu_regs.c`) and `Draw_FlushPrimQueue`
  (`draw/prim_queue.c`), 6 `lui` halves and the 6 integer-to-pointer casts
  of the source scan. Retail:

  ```
  80070D10  3C088007  lui   t0,0x8007
  80070D14  35080E0C  ori   t0,t0,0xe0c      ; queue table base
  80070D48  3C098007  lui   t1,0x8007
  80070D4C  35290E04  ori   t1,t1,0xe04      ; head offset
  80070D50  3C0A8007  lui   t2,0x8007
  80070D54  354A0E08  ori   t2,t2,0xe08      ; tail offset
  80070D6C..80070D80  the same three pairs (t0, t7, t8) in Draw_FlushPrimQueue
  ```

  The block they address, 0x80070E04..0x80070E50, is a 0x50-byte zero gap in
  `.text` after `Draw_PackPrimColor` (`pad` row at file offset 0x61604). A
  symbol reference cannot produce these words from C: cc1 2.7.2 emits
  `la reg,sym` for an address, which GNU as (and ASPSX) expand to
  `lui`/`addiu` with `%hi`/`%lo` (low instruction 0x25xxxxxx, not
  0x35xxxxxx), and MASPSX expands `li` only for a numeric operand
  (`expand_load_immediate`). `ori` with a `%lo` relocation would need hand
  assembly, which is not allowed, and would also be wrong for a block whose
  low half reaches 0x8000. Retail's `lui`/`ori` is therefore the shape of a
  numeric constant in the original source. Consequence: this is the one
  real shift hazard left in main. When `.text` grows before 0x80070E04 the
  block moves with the code but the six constants do not; a shifted build
  needs the block given a symbol and the two functions compiled with `la`
  (not byte-identical, so only in a non-matching build).
- `Render_LoadFontGlyphAlt` (`render/font.c`): `lui s0,0x8001` is the upper
  half of the multiplier 0x80010003 of a division by 0x7FFF. Not an address.
- `dtail_gp_post_dsfile` data words: 0x80017FFF twice (the halfword pair
  0x7FFF, 0x8001 inside the AKAO sound entries `g_AkaoLfoWave0`/`g_AkaoLfoWave1`)
  and 0x80010804 (one word of the random-looking table `D_8009BE78`). Not
  addresses.

### The catalogue

`linkers/USA/undefined_syms_manual.txt` held 1640 assignments. It now holds
only what main links: 10 SPU registers (hardware), the 2 LIBCARD patch
addresses (kernel) and the relative aliases below, each in a block whose
comment starts with the kind and names the owner. `shift_audit.py` groups
the file by these blocks and counts an assignment that is outside a block
of its own kind as `unowned` (ratchet 0). `undefined_addr_aliases.main.txt`
(generated) adds `D_A000DF80`, the LIBCARD patch entry, as a kernel address.

The 1224 names nothing linked against (address-form names of code and data
from earlier disassemblies, cheat-device and randomizer names in the
overlay windows, the unused kernel vectors `func_80000000`/`D_80000004`)
are in `configs/USA/symbol_catalogue.main.txt`. It is not a linker input;
`gen_expected.py` reads it for the retail disassembly's labels and
`shift_audit.py` reports it uncounted. 319 names outside the 2 MiB RAM
(`func_80302040` and the like, misdecoded jump targets) were dropped. A name
from the catalogue that code needs again goes into `sym.main.txt`, never
back into a linker file. No overlay took any of the moved names from main.

### Relative aliases: which can go

84 at the start of the wave. Each was tried mechanically (delete the old
name's declarations, rename its uses to the label, or `label[0]` for a
scalar view of an array label; accept only a byte-identical `make check`,
no new declaration conflict, then a clean rebuild of every main object and
all overlays, because the Makefile does not track header dependencies):

| result | aliases |
| --- | --- |
| removed (6) | `D_800A76F0` (u32[] view of `g_FieldRenderFlagTable`), `D_800B6A80` (`g_ScriptNodeActiveFlags`), `D_800BCF8E` (now `g_CameraClampedY[0]`), `D_800BCFEE` (now `g_ScreenTransitionState[0]`), `D_800C0DC0` (s32 view of `g_CdStreamMask`), `D_800C1F80` (s16[] view of `g_InvItemArray`) |
| unused, moved to the catalogue (1) | `g_GpuDmaState` |
| overlay spellings (17): the old name is what overlay C uses, with an overlay-local type; goes when the overlay declarations take main's name and type | `g_ViewMatrixSlot`, `g_PlayerEntity` (FieldActor view, 21 rooms), `D_800B0CD8`, `D_800B0DBA`, `D_800B0DBC`, `D_800B0DCD`, `D_800B0DD8`, `D_800B0E38`, `D_800B0E3C`, `D_800B0E4C`, `D_800B0E50`, `D_800B0E54`, `D_800B0E58`, `D_800B0E64`, `D_800BCF88`, `D_8009D1F4`, `D_8009D280` |
| type differs only in signedness or qualifier; the rename compiles but changes code (`lh`/`lhu`, a `volatile` reload) | `D_800BCFEA`, `D_800BCFF2` (s16 views of u16), `D_80095674`, `D_8009B784` (volatile views), `g_CdRingBufPtr` (u8 * view of `StHEADER *StRingAddr`), `D_8009D1E0`, `D_8009D090`, `g_EquippedWeaponSlot`, `g_EquippedArmorSlot` (scalar views of s8 arrays) |
| record views: a struct over a word or a block, used where the record form keeps retail's code (`pe1/battle_word_records.h` and friends); needs one struct type for the label | `g_PlayerEntityRecord`, `g_ActiveDrawSlotRecord`, `g_ActiveActorRecord`, `g_GameStateFlagsRecord`, `g_GameStateBlock`, `g_BattleResultFlagsRecord`, `g_FieldMoveLockRecord`, `g_PmTables`, `g_GpuDmaRegisterWindow`, `g_CursorRenderMetadataWindows`, `D_8009D1A8`, `g_AyaSaveState`, `g_InventoryRuntime`, `D_800B8AC0`, `D_800BC000`, `D_8009D2C8`, `D_800A76A0` |
| scalar/array and width views that do not compile as one declaration (one unit uses the word as a scalar, another indexes it, or reads a different width) | the other 34: `D_8009CE00`, `D_8009D098`, `D_8009D14C`, `D_800A76A4`, `D_800A76BC`, `g_ScriptVariableBank`, `D_800B0CE2`, `D_800B0E6C`, `D_800B89F8`, `D_800BD024`, `g_PadPort1Buffer`, `D_800C0D90`, `D_8009B788`, `D_8009B46C`, `g_AyaInventorySlotCount`, `D_8009D1CE`, `D_8009D1D0`, `D_8009D1E4`, `D_8009D1F0`, `D_8009D1F8`, `D_8009D228`, `D_8009D22C`, `D_8009D235`, `D_8009D238`, `D_8009D250`, `g_EntityCollisionWallParam`, `g_EntityCollisionWallSlot`, `D_8009D290`, `D_8009D29A`, `D_8009D29C`, `D_8009D2B0`, `D_8009D2D8`, `D_8009D2E8`, `D_8009D2F0` |

None of them is a fixed address: a relative alias moves with its label.
They are naming debt; the record and overlay groups are the ones worth a
dedicated pass.

## 10. A shifted build in an emulator (2026-10-13)

The link-level tests (`shift-test`, `shift-full-test`) prove that every word
that should move does. Whether a shifted build actually runs was tested by
booting one, with tools that write only to a scratch directory:

- `tools/scripts/shift_disc.py --pad P --out DIR` relinks main and all 190
  overlays with the pad, repacks `PE.IMG` (section 3.7), fits the executable
  into its disc file and writes a copy of the retail disc image with every
  changed Mode 2 Form 1 sector re-encoded (EDC, P and Q parity). The codec
  recomputes 64 untouched retail sectors identically, and `--pad 0
  --rewrite-all` rewrites 989 executable and 16310 `PE.IMG` sectors and
  gives back the retail `.bin` byte for byte. The executable cannot grow on
  the disc (its directory record and the files after it fix its sectors);
  a shift adds zero bytes at the end of the window block, which are cut and
  the header text size lowered by as much, so the BIOS loads as many bytes
  as from the retail file. `--insert-before INPUT` puts the pad before one
  input of the linker script instead of at the start of `.main`.
- `tools/scripts/emu_boot_test.py` drives DuckStation's regression runner
  (AI control port and GDB server) through a fixed input script: publisher
  screen, title, New Game, the opening movie, the street in front of
  Carnegie Hall, name entry, the first playable room, the aisle and lobby,
  the concert hall, the opera to the fire and the stage, 16 checkpoints over
  33834 frames. At each it saves the picture and reads the scene token and
  the program counter over GDB; `--compare` checks a run against the
  retail one pixel by pixel. Emulation is deterministic, so a shifted build
  that behaves like retail gives the same pictures on the same frames.

Bugs the emulator found, fixed (retail stays byte-identical):

- the PS-X EXE header kept the retail entry point 0x80072534: the BIOS
  started every shifted build inside the code before the startup (header
  now from `__SN_ENTRY_POINT`, section 3.6);
- block pointers in `PE.IMG` data outside the overlay slices: the second
  video player's file table and the credits line table
  (`linkers/USA/pe_img_pointers.txt`, section 3.7);
- overlay data that held addresses as numbers: menu_memcard's first video
  file table (bytes, so New Game played the next movie), its effect
  descriptors' work areas, and boot_display's credit tables (labels in the
  overlays' symbol files and `overlay_abi.txt`).

Results, every checkpoint compared with retail:

| pad | inserted | result |
| --- | --- | --- |
| 0x10 | start of `.main` | 16/16 identical pictures and scene tokens |
| 0x100 | start of `.main` | hangs during boot, no picture (PC at the BIOS vector 0xA0) |
| 0x800 | start of `.main` | 16/16 identical |
| 0x1FF0 (link maximum) | start of `.main` | crash during boot |
| 0x100, 0x1000, 0x1400 | before `draw/frame.c.o` | 16/16 identical |
| 0x1800, 0x1E00, 0x1FF0 | before `draw/frame.c.o` | crash during boot |
| 0x100 | start, primitive-queue units rebuilt with a symbol (scratch, not matching) | 16/16 identical |

What blocks, precisely:

1. The primitive-queue block (section 9, "Literals that stay"). With the pad
   at the start of `.main` the block moves while the six `lui`/`ori`
   constants of `Draw_InitGpuHwRegs` and `Draw_FlushPrimQueue` keep its
   retail address. At 0x100 that address holds the shifted
   `Draw_InitGpuHwRegs` itself: RAM 0x80070E40..0x80070E4C read 5, 3, 2, 1
   (the table it computes) where the executable has its loop, and the game
   never shows a picture (the program counter stays at the BIOS vector
   0xA0). At 0x10 and 0x800 the stores land in the
   tail of `Draw_PackPrimColor` and in `Render_DrawWireframeModel`, which
   this script never runs, so those runs pass by luck. Rebuilding the two
   units with a symbol for the block (a scratch experiment, not
   byte-identical) makes the 0x100 run identical to retail: on this path it
   is the only blocker for growth before 0x80070E04.
2. Run-time memory above the linked image. The link allows 0x1FF0 bytes
   (image end up to the initial `$sp`), but retail writes RAM above the
   image end at run time: after zeroing 0x801FE000..0x801FFF00 at the entry
   point, writes appear from the first screen on, up to 0x801FF8A8, 0x18A8
   bytes past the image end (in runs 0x800 apart), and they move with the
   block when it moves (pad 0x1000: the same pattern 0x1000 higher); the
   stack lives at the top (0x801FFF00 down). Which buffer it is was not
   identified (the runner's write watchpoints did not fire reliably). Pads of 0x1800 and more crash during boot; 0x1400 is
   the largest pad tested that plays the script. The real growth room is
   therefore below the 0x1FF0 the link check reports, and the check should
   count the run-time extent of the block's last buffers and the stack.
3. Not on this path, and fixed since: overlay data words that held their
   own function addresses as numbers (947 words in 36 overlays, listed by
   `shift_full_test.py`; Central Park, the museum, the Chrysler building,
   room_m022, scenes e09/e10). 945 were the rooms' class tables, seven
   methods per class; every room that links the room library's actor or
   window classes (120 overlays; scene_e22 keeps its table as asm code
   data) now links it from C as `g_RoomClassTable`
   (`room_lib/actor_class_table.c` and its two window variants, element
   type `RoomModuleHandler[7]`). The other two were the timed spawn record
   of room_m145 and m332. The room script also points at its own data with
   12-byte records (offset, `op << 24 | 8`, pointer; opcode 0x46 registers
   row 0 of the class table, 0x4B row 2, 0x49 row 4, 0x4A the window row):
   330 such pointers, and 7 sprite-record pointers in room_m065, sat in
   databins and are now data subsegments that name their target.
   `shift_full_test.py` reports no unrelocated function-address word left,
   and 1293 more overlay words move with the pad. Every row is in
   [overlay_data_pointers.csv](overlay_data_pointers.csv). Still numbers:
   17 room_m065 sprite-record pointers past the image end (run-time memory
   of the room, outside every symbol), and words that only look like
   addresses (122 halfword pairs such as 0x801C01E0 next to 0x00401010 in
   room image data, and 37 words 0x80010000).

Not verified: XA audio and sound in general (the runner has no audio
output; only the CD command stream was compared for the movies), whether
every movie plays (only the opening ones), any room or scene after the
stage, battles, saving and loading, disc 2, and real hardware.
