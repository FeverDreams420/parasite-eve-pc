# Contributing

This is a native C11 project. Preserve the intended game behavior and data
formats while improving the code for modern hosts. There is no byte-match
requirement, stock PS1 compiler requirement, PS1 checksum gate, source
manifest requirement or decompilation progress/debt ratchet.

## Development workflow

1. Use subsystem headers for shared declarations and ordinary C types and functions.
2. Replace platform operations with PsyZ APIs or explicit native adapters.
3. Add ported modules explicitly to the native CMake target when their dependencies work.
4. Build with `cmake -S . -B build` and `cmake --build build --parallel`.
5. Run `./build/pc/parasite-eve-pc --disc <Disc 1.bin>` in a graphical session for boot changes; `--smoke-test` checks PsyZ independently.
6. Validate changed game behavior with focused native tests or reproducible gameplay scenarios as modules become runnable.

The native target reads the disc, plays the intro and renders the title/menu.
A successful build does not imply that the game modules under `src/` compile
or that gameplay works. CI builds the target and tests the disc reader without
retail assets. Set `PE_TEST_DISC` when running CTest to exercise full movie
decoding, title selection, intro cancellation and frontend shutdown.
Use a fresh build directory when changing build structure or dependencies.

Do not preserve register assignments, instruction scheduling, padding,
linker order or code addresses for compiler matching. Assembly that performs
real work must be translated or replaced before its module is ported; do not
replace missing functionality with successful no-op stubs.

Serialized assets and save data retain their defined widths, offsets and
endianness. Host pointers must use pointer types, and host structures can be
redesigned independently of the original RAM layout. Validate layout assertions
against serialized formats rather than forcing host pointers into 32 bits.

Upstream is a source of optional fixes and knowledge. Imports must fit the
native architecture and behavior; they need no PS1 binary validation.

See [architecture](docs/ARCHITECTURE.md), [conventions](docs/CONVENTIONS.md),
[code organization](docs/CODE_ORGANIZATION.md) and
[the port plan](docs/PC_PORT_PLAN.md).

The full implementation roadmap and required automated acceptance evidence
are defined in [the port plan](docs/PC_PORT_PLAN.md). Feature completion requires
behavior assertions and a result for the affected content/platforms; a skipped
asset test or a passing source-only build cannot establish full-game readiness.
