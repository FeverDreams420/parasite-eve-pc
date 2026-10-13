# C conventions

Use C11 and the native compiler. Prefer clear functions, typed pointers,
subsystem headers and names that explain the role of a value. Existing
address-based names can be renamed as their meaning becomes clear.

Use fixed-width integer types for serialized data. Separate disk records
from host runtime structures; explicitly decode byte order, offsets and
indices before turning them into pointers. Do not assume `long` or a pointer
is 32 bits. Use `size_t` for sizes, `ptrdiff_t` for pointer differences and
`uintptr_t` only when an integer representation of a pointer is necessary.

Keep necessary packing and layout assertions for file formats. Original
code addresses, register assignments, compiler scheduling barriers,
per-file historical compiler flags and padding added to match instructions
are not part of the native design.

Hardware access belongs in PsyZ integration or a small platform adapter.
Existing MIPS/GTE routines still perform real work and must be implemented
in C or replaced by an equivalent API before integration. No-op substitutes
are appropriate only when the operation itself has no native meaning.

Validate behavior with native tests and gameplay scenarios. Compiler warnings,
sanitizers and tests should cover modules as they join the host build.
