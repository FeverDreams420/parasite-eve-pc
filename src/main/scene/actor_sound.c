#include "pe1/entity_animation.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/scene_assets.h"
#include "pe1/asset_find.h"

/* Original animation sound-trigger procedure, using the canonical actor. */
int Akao_LoadVoiceBank(BattleEntity *actor) {
    BattleEntity *actor_reg = actor;
    int frame;
    int step;
    int target;
    int i;
    int entry;
    Pe1GameState *state;
    int id;

    if (actor_reg == 0) {
        return -1;
    }

    state = &g_GameState;
    frame = ((u32)actor_reg->animFrame >> 16);
    step = actor_reg->animStep;
    target = actor_reg->animPrev.parts.integer;

    if (step > 0 && frame < target) {
        int next_frame;
        int extra;
        next_frame = frame + 1;
        extra = actor_reg->animLastFrame;
        frame = next_frame + extra;
    } else if (step < 0 && target < frame) {
        int next_frame;
        int extra;
        next_frame = frame - 1;
        extra = actor_reg->animLastFrame;
        frame = next_frame - extra;
    }

    if (actor_reg == ((BattleEntity *)g_PlayerEntity) && (g_GameStateFlags & 2) == 0 && (state->flags & 0x800000) == 0) {
        u16 *entry_ids;
        entry_ids = g_AkaoVoiceBankEntries[0].ids;
        entry = 0;
        for (i = 0; i < 4; entry_ids += 4, i++, entry++) {
            if (g_AkaoVoiceBankEntries[entry].index == actor_reg->actionMode) {
                int entry_frame = g_AkaoVoiceBankEntries[entry].frame;
                if ((step > 0 && entry_frame >= target && entry_frame < frame) ||
                    (step < 0 && frame < entry_frame && entry_frame <= target)) {
                    id = entry_ids[state->bank_state_12];
                    if (id != 0) {
                        Asset_Find08w(id, 0, actor_reg->posX.parts.integer,
                                      actor_reg->posY.parts.integer, actor_reg->posZ.parts.integer);
                    }
                }
            }
        }
    }

    i = 4;
    if (i >= state->bank_state_11 + 4) {
        return 0;
    }

    entry = 4;
    do {
        if (g_AkaoVoiceBankEntries[entry].group == actor_reg->modelIndex &&
            g_AkaoVoiceBankEntries[entry].set == actor_reg->teamId &&
            g_AkaoVoiceBankEntries[entry].index == actor_reg->actionMode) {
            int entry_frame = g_AkaoVoiceBankEntries[entry].frame;
            if ((step > 0 && entry_frame >= target && entry_frame < frame) ||
                (step < 0 && frame < entry_frame && entry_frame <= target)) {
                id = g_AkaoVoiceBankEntries[entry].ids[state->bank_state_12];
                if (id != 0) {
                    Asset_Find08w(id, 0, actor_reg->posX.parts.integer,
                                  actor_reg->posY.parts.integer, actor_reg->posZ.parts.integer);
                }
            }
        }

        entry++;
        i++;
    } while (i < state->bank_state_11 + 4);

    return 0;
}
