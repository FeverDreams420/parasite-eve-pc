#include "pe1/pad_state.h"
#include "pe1/scene_flags.h"

/* Original input state and callback filtering, separated from boot APIs. */
u32 g_FieldPadReleased;
u32 g_FieldPadPressed;
u32 g_FieldPadLastFrame;
u32 g_FieldPadBits;
u32 g_FieldPadBitsPrevious;

void Scene_CheckFlagBits(void *ctx, SceneFlagRecord **table, int *index_ptr)
{
    SceneFlagRecord *record;
    u32 flags;
    u32 mask_copy;
    int triggered;

    record = table[*index_ptr];
    triggered = 0;
    if (record != 0) {
        flags = record->flags;
    } else {
        flags = 0;
    }

    if (flags != 0) {
        do {
            void (*callback)(struct BattleEntity *, int *);
            u32 mask;
            u32 value;

            callback = record->callback;
            if (callback == 0) {
                return;
            }

            if (flags & 0x20) {
                callback(ctx, index_ptr);
                triggered = 1;
            } else if (flags & 1) {
                value = g_FieldPadBits;
                mask = record->mask_all;
                if ((value & mask) == mask) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 2) {
                        if (value & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 2) {
                u32 current;

                current = g_FieldPadBits;
                mask = record->mask_any;
                mask_copy = mask;
                if (current & mask_copy) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & mask) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & mask_copy) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 4) {
                if (g_FieldPadPressed & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            } else if (flags & 8) {
                if (g_FieldPadReleased & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            }

            if (((flags & 0x10) == 0) && (triggered != 0)) {
                return;
            }

            record++;
            flags = record->flags;
        } while (flags != 0);
    }

    if ((record != 0) && (record->callback != 0)) {
        record->callback(ctx, index_ptr);
    }
}
