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

/* Original game calls target the PS1 BIOS generator, not host libc RNG. */
int Psyz_Rand(void);
void Psyz_Srand(unsigned int seed);
#define rand Psyz_Rand
#define srand Psyz_Srand

/* Assert documented layouts of serialized data and legacy structures. */
#define PE1_STATIC_ASSERT(expr, name) \
    _Static_assert((expr), #name)

#define PE1_OFFSETOF(type, member) offsetof(type, member)

#endif
