#include "menu_memcard_image.h"

void MenuCard_InitFadeForNode(MemcardImageNode *state) {
    MemcardImageNode *node = ((MemcardImageNode *)g_MemcardActiveNodeHead);
    int kind = 7;

    state->y = 0xB4;
    state->field14 = 0;
    state->draw = 0;
    state->targetY = 0;
    while (node != 0) {
        if (node->kind == kind) {
            break;
        }
        node = node->next;
    }
    node->intensity = 0x100;
}

void MenuCard_UpdateFadeCounter(MemcardImageNode *state) {
    state->y = state->targetY + state->stepY;
    s32 shifted = (s32)((u32)state->stepY << 4);
    state->intensity = shifted >= 0 ? (s32)(256u - (u32)shifted) :
                                    (s32)((u32)shifted + 256u);
    state->stepY += state->stepY != 0;
    if (state->field14 && state->stepY == state->parameter)
        state->field14(state);
    if (state->stepY >= 16) state->freePending = 1;
}

void MenuCard_AdvanceFade(MemcardImageNode *state) {
    s32 level = state->intensity;

    if ((level <= 0 && state->targetY < 0) ||
        (level >= 0x100 && state->targetY > 0)) {
        state->targetY = 0;
    }
    state->intensity += state->targetY;
}

void MenuCard_UpdateFadeDisplay(MemcardImageNode *state) {
    {
        s32 velocity = state->targetY;
        s32 increase = 0;
        s32 level;

        if (velocity != 0) {
            increase = velocity < 0x54;
        }
        level = state->intensity;
        velocity += increase;
        state->targetY = velocity;
        if (level < 0x100) {
            level += (s32)((u32)state->stepY << 3);
        }
        state->intensity = level;
        if (level == 0 || level == 0x100) {
            state->stepY = 0;
        }
    }
    {
        s32 current = state->targetY;
        s32 top = 0x3C - current;

        if (top >= 0) {
            top = current + 0x50;
        } else {
            top = 0x8C;
        }
        state->y = top;
    }
    {
        s32 current = state->targetY;
        s32 delta = 0x54 - current;
        s32 bottom;

        if (delta >= 0x18) {
            bottom = delta;
        } else {
            bottom = 0x18;
        }
        state->height = bottom;
    }
}

void MenuCard_DecreaseFade(MemcardImageNode *state) {
    s32 level = state->intensity - 0x10;

    if (level < 0) {
        level = 0;
    }
    state->intensity = level;
    if (level == 0) {
        state->stepY = 0;
        state->targetY = 0;
    }
}

void MenuCard_CreateFadeImageNode(MemcardImageNode *unused) {
    (void)unused;
    MenuCard_CreateImageNode(2);
}

void MenuCard_AttachFadePayload(MemcardImageNode *unused) {
    (void)unused;
    MemcardImageNode *node;
    int kind;

    node = ((MemcardImageNode *)g_MemcardActiveNodeHead);
    kind = 2;
    while (node != 0) {
        if (node->kind == kind) {
            break;
        }
        node = node->next;
    }

    node->draw = MenuCard_StepFadeToImages;
}

void MenuCard_CreateImageGroup(void);

void MenuCard_StepFadeToImages(MemcardImageNode *state) {
    int step_value;

    step_value = state->intensity - 0x10;
    state->intensity = step_value;

    if (step_value == 0x80) {
        MenuCard_CreateImageGroup();
    }

    if (state->intensity == 0) {
        state->freePending = 1;
    }
}
