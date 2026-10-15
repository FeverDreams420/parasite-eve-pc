#include "pe1/geom_state.h"
#include "pe1/room_fx.h"
#include "pe1/fieldfx_task.h"
#include "pe1/pm.h"
#include "menu_memcard_glint.h"
#include "menu_memcard_link.h"
#include "menu_memcard_pulse.h"
#include "menu_memcard_trail.h"

s32 MenuCard_PulseImage(int mode, void *data, intptr_t argument) {
    MemcardPulseState *state=data;
    (void)argument;
    GteRotation rotation;
    RenderColor color;
    s32 angle;
    s32 scale;
    s32 palette;
    s16 timer;
    s32 twist;
    void *track;
    switch (mode) {
    case 1:
        if (state->phase != 0) return 0;
        timer = state->timer + 1;
        state->timer = timer;
        if (timer < 16) return 0;
        return 1;
    case 2:
        if (state->phase != 0) return 0;
        angle = state->timer * 64;
        scale = rsin(angle) + 2048;
        rcos(angle);
        rotation.x = 0; rotation.y = 0; twist = state->timer;
        track = g_MenuCardPulseColorTrack;
        rotation.flags = 0;
        rotation.z = twist * 32;
        FieldFx_SampleColorTrack(track, &color, state->timer * 2);
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
        FieldFx_DrawShapeQuads(&state->position, &rotation, scale, scale, 130,
                     GetClut(48, palette), 1, 128, &color);
        break;
    }
    return 0;
}

s32 MenuCard_LinkEffect(int mode, void *data, intptr_t argument) {
    MemcardLinkState *state=data;
    (void)argument;
    GteShortVector unused;
    GteShortVector first;
    GteShortVector second;
    MemcardJointOffset offset = g_MenuCardHandOffset;
    RenderColor color;
    MemcardLinkColor color1 = g_MenuCardLinkColor1;
    MemcardLinkColor color2 = g_MenuCardLinkColor2;
    MemcardJointOffset *offsetPtr;
    switch (mode) {
    case 0: {
        s32 opacity;
        RoomFxTransformOwner *entity;
        GteShortVector *firstPtr;
        offsetPtr = &offset;
        opacity = 128; entity = (RoomFxTransformOwner *)(void *)D_8009D254;
        firstPtr = &first;
        state->opacity = opacity; state->phase = 0; state->timer = 0;
        FieldFx_TransformMatrixPoint(entity, g_MenuCardLinkJointA, offsetPtr, firstPtr);
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, g_MenuCardLinkJointB, &offset, &second);
        FieldFx_DrawHistoryTrail(&first, &second, 1016, (unsigned char *)&color2, (unsigned char *)&color1, 128, state->history, 1);
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 20, 22,
                             MenuCard_PulseImage);
    }
    case 1:
        {
            s16 timer = state->timer + 1;
            RoomFxTransformOwner *entity = (RoomFxTransformOwner *)(void *)D_8009D254;

            state->timer = timer;
            FieldFx_TransformMatrixPoint(entity, g_MenuCardLinkJointB, &offset, &unused);
        }
        if (state->timer >= 23 && state->opacity > 0) state->opacity -= 8;
        if (state->timer < 22) return 0;
        return 2;
    case 2:
        if (state->opacity >= 0) {
            FieldFx_SampleColorTrack(g_MenuCardPulseColorTrack, &color, state->timer);
            FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, g_MenuCardLinkJointA, &offset, &first);
            FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, g_MenuCardLinkJointB, &offset, &second);
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_DrawHistoryTrail(&first, &second, 16, (unsigned char *)&color2, (unsigned char *)&color1,
                         state->opacity, state->history, 1);
        }
        {
            u16 tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot1];
            g_FieldFxSpriteParams.palette = 1;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        FieldFx_EnsureEffectTexturePage(1);
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 16;
        break;
    }
    return 0;
}

s32 MenuCard_CosineImage(int mode, void *data, intptr_t argument) {
    MemcardPulseState *state=data;
    (void)argument;
    GteRotation rotation;
    s32 angle, scale, palette;
    s32 intensity;
    s32 cosine;
    s32 index;
    s32 four;
    s16 timer;
    s32 twist;
    switch (mode) {
    case 1:
        if (state->phase != 0) return 0;
        timer = state->timer + 1;
        state->timer = timer;
        if (timer < 12) return 0;
        return 1;
    case 2:
        if (state->phase != 0) return 0;
        angle = (state->timer * 1024) / 12;
        scale = rcos(angle) + 2048;
        cosine = rcos(angle);
        intensity = cosine / 32;
        rotation.x = 0; rotation.y = 0;
        twist = state->timer;
        index = g_FieldFxSpriteParams.palette;
        four = 4;
        rotation.flags = 0;
        rotation.z = twist * 32;
        palette = g_FieldFxPaletteTable[index];
        if (index == four && g_FieldFxAltPaletteFlag != 0) palette += 7;
        else palette += 3;
        FieldFx_DrawShapeQuads(&state->position, &rotation, scale, scale, 66,
                     GetClut(0, palette), 1, intensity, &g_MemcardCosineColor);
        break;
    }
    return 0;
}

s32 MenuCard_TrailEffect(int mode, void *data, intptr_t argument) {
    MemcardTrailState *state=data;
    (void)argument;
    GteShortVector position;
    MemcardJointOffset offset = g_MenuCardHandOffset;
    RenderColor color;
    s32 scale, palette;
    s32 index;
    s32 four;
    switch(mode) {
    case 0:
        state->attachment = 19; state->phase = 0; state->timer = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 20, 24,
                             MenuCard_CosineImage);
    case 1:
        state->timer++;
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, state->attachment, &offset, &position);
        if (state->timer < 33 && (state->timer & 1)) {
            MemcardPulseState *image = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (image) {
                image->position.x = position.x;
                image->position.y = position.y;
                image->position.z = position.z;
                image->phase = 0; image->timer = 0;
            }
        }
        if (state->timer >= 32) return 2;
        break;
    case 2:
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, state->attachment, &offset, &position);
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32; g_FieldFxSpriteParams.extent_y = 32;
        {
            u16 tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot9];
            g_FieldFxSpriteParams.palette = 1;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        FieldFx_EnsureEffectTexturePage(1);
        g_FieldFxSpriteParams.parameter06 = 1;
        g_FieldFxSpriteParams.parameter0A = 5;
        g_FieldFxSpriteParams.depth = 32;
        if (state->timer < 33) {
            FieldFx_SampleColorTrack(g_MenuCardTrailColorTrack, &color, state->timer);
            scale = rsin(state->timer * 64) + 4096;
            index = g_FieldFxSpriteParams.palette;

            four = 4;

            palette = g_FieldFxPaletteTable[index];
            if (index == four && g_FieldFxAltPaletteFlag) palette += 8;
            else palette += 4;
            FieldFx_DrawShapeQuads(&position, 0, scale, scale, 64, GetClut(0,palette), 1, 128, &color);
            index = g_FieldFxSpriteParams.palette;
            four = 4;

            palette = g_FieldFxPaletteTable[index];
            if (index == four && g_FieldFxAltPaletteFlag) palette += 7;
            else palette += 3;
            FieldFx_DrawShapeQuads(&position, 0, scale, scale, 70, GetClut(0,palette), 1, 128, &color);
            g_MemcardCosineColor.r = color.r; g_MemcardCosineColor.g = color.g; g_MemcardCosineColor.b = color.b;
        } else {
            g_MemcardCosineColor = (RenderColor){0};
        }
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 16;
        break;
    }
    return 0;
}

s32 MenuCard_SineRotatingImage(int mode, void *data, intptr_t argument) {
    MemcardPulseState *state=data;
    (void)argument;
    GteRotation rotation;
    s32 angle, scale;
    s32 palette;
    s32 intensity;
    s32 cosine;
    s32 index;
    s32 four;
    s16 timer;
    s32 twist;
    switch (mode) {
    case 1:
        if (state->phase != 0) return 0;
        timer = state->timer + 1;
        state->timer = timer;
        if (timer < 12) return 0;
        return 1;
    case 2:
        if (state->phase != 0) return 0;
        angle = (state->timer * 1024) / 12;
        scale = rsin(angle) + 4096;
        cosine = rcos(angle);
        if (cosine < 0) cosine += 31;
        rotation.x = state->rotation.x;
        index = g_FieldFxSpriteParams.palette;
        rotation.y = state->rotation.y;
        rotation.z = state->rotation.z;
        intensity = cosine >> 5;
        twist = state->timer;
        rotation.flags = 1;
        rotation.z = twist * 50;
        {
            four = 4;
            palette = g_FieldFxPaletteTable[index];
        }
        if (index == four && g_FieldFxAltPaletteFlag != 0) palette += 7;
        else palette += 3;
        FieldFx_DrawShapeQuads(&state->position, &rotation, scale, scale, 68,
                     GetClut(0, palette), 1, intensity, &g_MemcardSineColor);
        break;
    }
    return 0;
}

s32 MenuCard_SineTrailEffect(int mode, void *data, intptr_t argument) {
    MemcardTrailState *state=data;
    (void)argument;
    GteShortVector position;
    MemcardJointOffset offset = g_MenuCardHandOffset;
    RenderColor color;
    RenderMatrixSlot *matrixSlot;
    s32 scale, kind, palette;

    switch (mode) {
    case 0:
        state->attachment = 25;
        state->phase = 0;
        state->timer = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 20, 24,
                             MenuCard_SineRotatingImage);
    case 1:
        state->timer++;
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, state->attachment, &offset, &position);
        if (state->timer < 33 && (state->timer & 1)) {
            MemcardPulseState *image = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (image) {
                image->position.x = position.x;
                image->position.y = position.y;
                image->position.z = position.z;
                FieldFx_GetTransformLookAngles((RoomFxTransformOwner *)(void *)D_8009D254, 0, (GteShortVector *)&image->rotation);
                image->phase = 0;
                image->timer = 0;
            }
        }
        if (state->timer >= 32) {
            return 2;
        }
        break;
    case 2:
        matrixSlot = &g_ViewMatrixSlot;
        gte_ldrotmatrix(matrixSlot->value);
        gte_ldtransmatrix(matrixSlot->value);
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)(void *)D_8009D254, state->attachment, &offset, &position);
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        {
            u16 tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot9];
            g_FieldFxSpriteParams.palette = 1;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        FieldFx_EnsureEffectTexturePage(1);
        g_FieldFxSpriteParams.parameter06 = 1;
        g_FieldFxSpriteParams.parameter0A = 5;
        g_FieldFxSpriteParams.depth = 32;
        if (state->timer < 33) {
            FieldFx_SampleColorTrack(g_MenuCardSineTrailColorTrack, &color, state->timer);
            scale = rsin(state->timer * 64) / 2 + 2048;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            FieldFx_DrawShapeQuads(&position, 0, scale, scale, 64,
                          GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag) ? palette + 8 : palette + 4),
                          1, 128, &color);
            g_MemcardSineColor.r = color.r;
            g_MemcardSineColor.g = color.g;
            g_MemcardSineColor.b = color.b;
        } else {
            g_MemcardSineColor = (RenderColor){0};
        }
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 16;
        break;
    }
    return 0;
}

/* Origin callbacks of the glint effect descriptors; every effect is
 * anchored at the shared glint origin. */
intptr_t MenuCard_GetGlintOriginA(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

intptr_t MenuCard_GetGlintOriginB(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

intptr_t MenuCard_GetGlintOriginC(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

intptr_t MenuCard_GetGlintOriginD(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

intptr_t MenuCard_GetGlintOriginE(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

intptr_t MenuCard_GetGlintOriginF(int mode, intptr_t a, intptr_t b, intptr_t c) {
    (void)mode;(void)a;(void)b;(void)c;
    return (intptr_t)&g_MemcardGlintOrigin;
}

/* Update hook of the cross flash: keeps it alive while the flash raises the
 * flag each frame and clears the flag for the next frame. */
int MenuCard_ConsumeCrossFlashFlag(int mode, void *state, intptr_t argument) {
    (void)state;(void)argument;
    if (mode == 1) {
        if (g_MenuCardCrossFlashRaised == 0) return 1;
        g_MenuCardCrossFlashRaised = 0;
    }
    return 0;
}

/* Original mutable bank data. */
RenderColor g_MemcardCosineColor, g_MemcardSineColor;
s16 g_MenuCardLinkJointA=25, g_MenuCardLinkJointB=26;
MemcardLinkColor g_MenuCardLinkColor1 = {{210,160,240,0}};
MemcardLinkColor g_MenuCardLinkColor2 = {{250,10,10,0}};
_Alignas(s32) u8 g_MenuCardPulseColorTrack[40] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc8,0xa0,0xc8,0x0c,0x00,0x00,0x00,0x00,
    0xb4,0x50,0xc8,0x0c,0x00,0x00,0x00,0x00,0x80,0x00,0xb4,0x08,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
_Alignas(s32) u8 g_MenuCardTrailColorTrack[48] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x08,0x00,0x00,0x00,0x00,
    0xc8,0xa0,0xc8,0x08,0x00,0x00,0x00,0x00,0xb4,0x50,0xc8,0x08,0x00,0x00,0x00,0x00,
    0x80,0x00,0xb4,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
_Alignas(s32) u8 g_MenuCardSineTrailColorTrack[48] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x08,0x00,0x00,0x00,0x00,
    0xc8,0xa0,0xc8,0x08,0x00,0x00,0x00,0x00,0xb4,0x50,0xc8,0x08,0x00,0x00,0x00,0x00,
    0x80,0x00,0xb4,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* Original program 111: descriptor 801F1D8C, script 801F1D38. */
static u16 menu_card_script_111[] = {1,6,26,2,2,0,4,5,0,2,0,0,2,1,0,0,0,0};
FieldFxTaskProgram g_FieldFxTaskProgramCmd111 = {
    .table={.callbacks={MenuCard_GlintBurstController,MenuCard_SpinBurstController,MenuCard_LinkEffect},
            .sizes={8,16,280,0,0,0,0,0}},
    .initialize=MenuCard_GetGlintOriginD,
    .script=menu_card_script_111,
};

/* Original program 112: descriptor 801F1E18, script 801F1DC4. */
static u16 menu_card_script_112[] = {1,6,26,2,2,0,4,5,0,2,0,0,2,1,0,0,0,0};
FieldFxTaskProgram g_FieldFxTaskProgramCmd112 = {
    .table={.callbacks={MenuCard_GlintBurstController,MenuCard_SpinBurstController,MenuCard_TrailEffect},
            .sizes={8,16,8,0,0,0,0,0}},
    .initialize=MenuCard_GetGlintOriginE,
    .script=menu_card_script_112,
};

/* Original program 113: descriptor 801F1EA4, script 801F1E50. */
static u16 menu_card_script_113[] = {1,6,26,2,2,0,4,5,0,2,0,0,2,1,0,0,0,0};
FieldFxTaskProgram g_FieldFxTaskProgramCmd113 = {
    .table={.callbacks={MenuCard_GlintBurstController,MenuCard_SpinBurstController,MenuCard_SineTrailEffect},
            .sizes={8,16,8,0,0,0,0,0}},
    .initialize=MenuCard_GetGlintOriginF,
    .script=menu_card_script_113,
};

/* Original program 114: descriptor 801F1EF0, script 801F1EDC. */
static u16 menu_card_script_114[] = {1,6,29,2,0,0,0,0,0};
FieldFxTaskProgram g_FieldFxTaskProgramCmd114 = {
    .table={.callbacks={MenuCard_ConsumeCrossFlashFlag},
            .sizes={0,0,0,0,0,0,0,0}},
    .script=menu_card_script_114,
};
