#ifndef PE1_COMMON_H
#define PE1_COMMON_H

#include <stddef.h>
#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float f32;

/* Assert documented layouts of serialized data and legacy structures. */
#define PE1_STATIC_ASSERT(expr, name) \
    _Static_assert((expr), #name)

#define PE1_OFFSETOF(type, member) offsetof(type, member)

#endif
