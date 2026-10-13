# Code organization

`pc/` owns the host entry point and runtime integration. `src/main/` groups
game logic by subsystem; `include/pe1/` contains shared subsystem headers.
Add sources to CMake explicitly as they become portable and runnable.

`src/overlays/` holds room, scene and effect code. Native modules should use
explicit registration, namespaced symbols and clear initialization/reset
boundaries. They do not need fixed RAM windows, PS1 linker scripts or binary
repacking. Preserve the identifiers and offsets used by game assets while
separating them from host addresses.

`src/main/psyq/` retains legacy runtime code as functional reference for
porting. Compare its behavior with PsyZ before replacing each dependency;
game-specific CD/AKAO behavior may need adapters. Do not link the legacy SDK
implementations wholesale alongside PsyZ.

Move or merge translation units according to responsibilities and ownership.
There is no manifest, symbol-order or original compiler boundary to preserve.
Use subsystem headers for cross-module declarations and ordinary definitions
for runtime state. Remove address aliases as the affected state is ported.

Data-format research under `docs/` is reference material, not a build input
or a fixed-address requirement. Historical source remains available in Git.
