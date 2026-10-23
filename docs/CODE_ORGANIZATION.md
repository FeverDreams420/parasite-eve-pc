# Code organization

Repair complete original modules in their existing files under `src/main/`
and `src/overlays/`; `include/pe1/` contains shared subsystem headers. Do not
create new files or extract selected functions while restoring the original
game. Add the existing whole modules to CMake as they become portable.

The abandoned `pc/` directory and its dependent test fixtures have been removed.
Do not recreate or relocate its parallel implementations. Necessary host changes
belong in the existing original modules or the PsyZ fork.

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

The original actor and effect workspaces have multiple typed layout views of
the same storage. GCC/Clang builds of the original modules and their fixtures
must preserve that aliasing (`-fno-strict-aliasing`, set by root CMake after
configuring PsyZ). Keep this contract for manual builds too. The SDK and SDL
retain their own compiler policies. Do not hide view failures by weakening tests.

Keep original translation units intact; do not move functions into new files.
There is no binary-layout or original compiler requirement to preserve.
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
