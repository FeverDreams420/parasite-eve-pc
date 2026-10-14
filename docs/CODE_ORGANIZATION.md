# Code organization

`pc/` owns the host entry point and runtime integration. `src/main/` groups
game logic by subsystem; `include/pe1/` contains shared subsystem headers.
Add sources to CMake explicitly as they become portable and runnable.

`src/overlays/` holds room, scene and effect code. Native modules should use
static linking, explicit registration, namespaced symbols and clear
initialization/reset boundaries. They do not need fixed RAM windows, PS1 linker scripts or binary
repacking. Preserve the identifiers and offsets used by game assets while
separating them from host addresses.

`src/main/psyq/` retains legacy runtime code as functional reference for
porting. PS1 APIs may be removed, simplified or replaced with native C when
gameplay stays faithful. Image-backed asset reads replace CD-drive protocols;
AKAO still needs equivalent audio behavior. Use the maintained
`khasinski/parasite-eve-psyz` source fork for needed generic runtime features.
Do not link legacy SDK implementations wholesale alongside PsyZ.

Move or merge translation units according to responsibilities and ownership.
There is no manifest, symbol-order or original compiler boundary to preserve.
Use subsystem headers for cross-module declarations and ordinary definitions
for runtime state. Remove address aliases as the affected state is ported.

Data-format research under `docs/` is reference material, not a build input
or a fixed-address requirement. Historical source remains available in Git.

Scratchpad work and stack switching become ordinary local/context buffers and
the host stack. Preserve data lifetimes and reset behavior, not PS1 addresses.
Both CUE/BIN images will be mounted as one logical asset set, with explicit
source identity for colliding names/LBAs and automatic selection by game content.
Hi-res uses PsyZ's existing internal resolution scaling from the first scene
integration; high-FPS presentation is a later stage of the roadmap.
