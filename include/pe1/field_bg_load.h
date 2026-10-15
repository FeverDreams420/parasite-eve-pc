#ifndef PE1_FIELD_BG_LOAD_H
#define PE1_FIELD_BG_LOAD_H

#include "common.h"
#include "pe1/psyq_gpu.h"

/* Background texture slots: VRAM page origin and CLUT origin, with the
 * packed tpage/clut words the loader derives from them. */
typedef struct FieldBgTextureSlot {
    u16 x, y;
    u16 clut_x, clut_y;
    u16 tpage, clut;
    u32 reserved;
} FieldBgTextureSlot;

/* Raw image upload record: total length in bytes, VRAM rectangle, pixels. */
typedef struct FieldBgImageRecord {
    u32 length;
    RECT rect;
    u32 pixels[1];
} FieldBgImageRecord;

/* libgpu getTPage/getClut packing (texture mode 0, semi-transparency 1). */
#define BG_SLOT_TPAGE(x, y) \
    ((0 << 7) | (1 << 5) | (((y) & 0x100) >> 4) | (((x) & 0x3ff) >> 6) | (((y) & 0x200) << 2))
#define BG_SLOT_CLUT(x, y) (((y) << 6) | (((x) >> 4) & 0x3f))

/* The font and field loader share the same VRAM slots, viewed four bytes
 * apart in the original resident data. Keep their derived tpage/CLUT shared. */
typedef struct TextboxFontPage {
    u32 reserved;
    u16 textureX, textureY, paletteX, paletteY;
    u16 tpage, clut;
} TextboxFontPage;
typedef union FieldFontTextureState {
    struct { TextboxFontPage pages[4]; u32 tail; } font;
    struct { u32 prefix; FieldBgTextureSlot slots[4]; } background;
} FieldFontTextureState;
extern FieldFontTextureState g_FieldFontTextureState;
#define g_FieldBgTextureSlots (g_FieldFontTextureState.background.slots)
#define g_TextboxFontPages (g_FieldFontTextureState.font.pages)
PE1_STATIC_ASSERT(sizeof(FieldFontTextureState) == 68, field_font_texture_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFontTextureState, background.slots) == 4, field_font_slots_offset);

/* PE.IMG start/end sector table (consecutive entries bound one chunk). */
extern u16 g_PeImageSectorTable[];

void Akao_Cmd_F1(void);
void ResetGraph(int mode);
void Battle_DrawHPBar(void);

int Scene_LoadFieldBg(void);

#endif
