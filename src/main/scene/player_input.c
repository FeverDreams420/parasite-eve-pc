#include "pe1/geom_state.h"
#include "pe1/battle_runtime.h"
#include "pe1/field_movement.h"
#include "pe1/gte.h"
#include "pe1/pad_state.h"
#include "pe1/player_entity.h"

/* Field player movement state handlers: idle, walk, run, turn-in-place and
 * the idle resync. All five are contiguous, compile under the default
 * profile and share the pad/analog and camera-angle globals. */

void Scene_Init(BattleEntity *entity, int *actionState) {
    if (g_GameStateFlags & 2) {
        Scene_SyncEntityAction(entity, actionState);
        return;
    }
    if (*actionState != 0x15) {
        entity->motionX = 0;
        entity->motionY = 0;
        entity->motionZ = 0;
        Entity_SetActionMode(entity, 0x15);
        *actionState = 0x15;
    }
}

void Scene_UpdatePlayerEntity(BattleEntity *entity, int *actionState)
{
    GteVector movement = {0};
    int speed;
    int diagonal;
    int facing;
    u32 pad;

    if (g_GameStateFlags & 2) {
        Scene_UpdateEntityFacing(entity, actionState);
        return;
    }
    if (*actionState != 22) {
        Entity_SetActionMode(entity, 22);
        *actionState = 22;
    }
    speed = Math_FixedMul(0x50000, entity->moveFactor);
    speed = Math_FixedMul(speed, entity->moveSpeed << 4);
    if ((g_PadPackets[0].header & 0xF000) == 0x7000) {
        int angle;
        angle = ratan2(g_PadPackets[0].leftY - 128, g_PadPackets[0].leftX - 128) - 0x400;
        if (angle < 0) angle += 0x1000;
        entity->facingAngle = g_GeomScroll.state.position.baseAngleAnalog + angle;
        movement.x = Math_FixedMul(-speed, rsin(angle) * 16);
        movement.y = 0;
        movement.z = Math_FixedMul(-speed, rcos(angle) * 16);
    } else {
        diagonal = Math_FixedMul(speed, 0xB504);
        pad = g_FieldPadBits;
        entity->facingAngle = g_GeomScroll.state.position.baseAngleDigital;
        if (pad & FIELD_PAD_DOWN) {
            if (pad & FIELD_PAD_LEFT) {
                entity->facingAngle += 0x600;
                movement.x = -diagonal;
                movement.y = 0;
                movement.z = diagonal;
            } else if (pad & FIELD_PAD_RIGHT) {
                entity->facingAngle += 0xA00;
                movement.x = diagonal;
                movement.y = 0;
                movement.z = diagonal;
            } else {
                entity->facingAngle += 0x800;
                movement.x = 0;
                movement.y = 0;
                movement.z = speed;
            }
        } else if (pad & FIELD_PAD_UP) {
            if (pad & FIELD_PAD_LEFT) {
                entity->facingAngle += 0x200;
                movement.x = -diagonal;
                movement.y = 0;
                movement.z = -diagonal;
            } else if (pad & FIELD_PAD_RIGHT) {
                entity->facingAngle += 0xE00;
                movement.x = diagonal;
                movement.y = 0;
                movement.z = -diagonal;
            } else {
                entity->facingAngle = g_GeomScroll.state.position.baseAngleDigital;
                movement.x = 0;
                movement.y = 0;
                movement.z = -speed;
            }
        } else if (pad & FIELD_PAD_LEFT) {
            entity->facingAngle += 0x400;
            movement.x = -speed;
            movement.y = 0;
            movement.z = 0;
        } else if (pad & FIELD_PAD_RIGHT) {
            entity->facingAngle += 0xC00;
            movement.x = speed;
            movement.y = 0;
            movement.z = 0;
        }
    }
    facing = entity->facingAngle;
    if (((BattleEntity *)g_PlayerEntity) && (D_8009D2E8 & FIELD_MOVE_CONFUSED)) {
        u32 stateFlags = ((Combatant *)((BattleEntity *)g_PlayerEntity)->core)->stateFlags;

        facing += 0x800;
        facing += (stateFlags >> 7) & 0xC00;
    }
    facing &= 0xFFF;
    entity->facingAngle = facing;
    ApplyMatrixLV(&g_GeomScroll.state.position.cameraBasis, &movement, (GteVector *)&entity->motionX);
    if (D_8009D2E8 & FIELD_MOVE_CONFUSED) {
        entity->facingAngle = (0x1000 - (u16)entity->facingAngle) & 0xFFF;
        entity->motionX = -entity->motionX;
    }
}

void Scene_UpdateEntityFacing(BattleEntity *entity, int *actionState)
{
    GteVector movement = {0};
    int speed;
    int diagonal;
    int facing;
    u32 pad;

    speed = AYA_MOVE_SPEED_NORMAL;
    if (g_GameStateFlags & 2) {
        int flags = ((Combatant *)entity->core)->stateFlags;
        if ((flags & AYA_STIFFNESS_MASK) == AYA_STIFFNESS_SLOW) {
            speed = AYA_MOVE_SPEED_STIFF;
        } else if (flags & AYA_HASTE) {
            speed = AYA_MOVE_SPEED_HASTE;
        }
        {
            u8 mode;
            mode = (u8)((Combatant *)entity->core)->actionMode13;
            if (*actionState != mode) {
                Entity_SetActionMode(entity, mode);
                *actionState = (u8)((Combatant *)entity->core)->actionMode13;
            }
        }
    } else if (*actionState != 23) {
        Entity_SetActionMode(entity, 23);
        *actionState = 23;
    }
    speed = Math_FixedMul(speed, entity->moveFactor);
    speed = Math_FixedMul(speed, entity->moveSpeed << 4);
    if ((g_PadPackets[0].header & 0xF000) == 0x7000) {
        int angle;
        angle = ratan2(g_PadPackets[0].leftY - 128, g_PadPackets[0].leftX - 128) - 0x400;
        if (angle < 0) angle += 0x1000;
        entity->facingAngle = g_GeomScroll.state.position.baseAngleAnalog + angle;
        movement.x = Math_FixedMul(-speed, rsin(angle) * 16);
        movement.y = 0;
        movement.z = Math_FixedMul(-speed, rcos(angle) * 16);
    } else {
        diagonal = Math_FixedMul(speed, 0xB504);
        pad = g_FieldPadBits;
        entity->facingAngle = g_GeomScroll.state.position.baseAngleDigital;
        if (pad & FIELD_PAD_DOWN) {
            if (pad & FIELD_PAD_LEFT) {
                entity->facingAngle += 0x600;
                movement.x = -diagonal;
                movement.y = 0;
                movement.z = diagonal;
            } else if (pad & FIELD_PAD_RIGHT) {
                entity->facingAngle += 0xA00;
                movement.x = diagonal;
                movement.y = 0;
                movement.z = diagonal;
            } else {
                entity->facingAngle += 0x800;
                movement.x = 0;
                movement.y = 0;
                movement.z = speed;
            }
        } else if (pad & FIELD_PAD_UP) {
            if (pad & FIELD_PAD_LEFT) {
                entity->facingAngle += 0x200;
                movement.x = -diagonal;
                movement.y = 0;
                movement.z = -diagonal;
            } else if (pad & FIELD_PAD_RIGHT) {
                entity->facingAngle += 0xE00;
                movement.x = diagonal;
                movement.y = 0;
                movement.z = -diagonal;
            } else {
                entity->facingAngle = g_GeomScroll.state.position.baseAngleDigital;
                movement.x = 0;
                movement.y = 0;
                movement.z = -speed;
            }
        } else if (pad & FIELD_PAD_LEFT) {
            entity->facingAngle += 0x400;
            movement.x = -speed;
            movement.y = 0;
            movement.z = 0;
        } else if (pad & FIELD_PAD_RIGHT) {
            entity->facingAngle += 0xC00;
            movement.x = speed;
            movement.y = 0;
            movement.z = 0;
        }
    }
    facing = entity->facingAngle;
    if (((BattleEntity *)g_PlayerEntity) && (D_8009D2E8 & FIELD_MOVE_CONFUSED)) {
        u32 stateFlags = ((Combatant *)((BattleEntity *)g_PlayerEntity)->core)->stateFlags;

        facing += 0x800;
        facing += (stateFlags >> 7) & 0xC00;
    }
    facing &= 0xFFF;
    entity->facingAngle = facing;
    ApplyMatrixLV(&g_GeomScroll.state.position.cameraBasis, &movement, (GteVector *)&entity->motionX);
    if (D_8009D2E8 & FIELD_MOVE_CONFUSED) {
        entity->facingAngle = (0x1000 - (u16)entity->facingAngle) & 0xFFF;
        entity->motionX = -entity->motionX;
    }
}

void Scene_SyncEntityAction(BattleEntity *entity, int *actionState) {
    u8 mode;

    if (!(g_GameStateFlags & 2)) {
        if (*actionState != 0x15) {
            entity->motionX = 0;
            entity->motionY = 0;
            entity->motionZ = 0;
            Entity_SetActionMode(entity, 0x15);
            *actionState = 0x15;
        }
    } else {
        mode = ((Combatant *)entity->core)->actionMode12;
        if (*actionState != mode) {
            Entity_SetAction(entity, mode);
            entity->motionX = 0;
            entity->motionY = 0;
            entity->motionZ = 0;
            *actionState = ((Combatant *)entity->core)->actionMode12;
        }
    }
}

void Scene_UpdateEntityFacingFromPad(BattleEntity *entity) {
    int angle;
    Entity_SetAction(entity, 0x11);
    if ((g_PadPackets[0].header & 0xF000) == 0x7000) {
        angle = ratan2(g_PadPackets[0].leftY - 128, g_PadPackets[0].leftX - 128) - 0x400;
        /* Keep the wrapped-angle correction based on the adjusted value. */
        if (angle < 0) angle += 0x1000;
        angle += g_GeomScroll.state.position.baseAngleAnalog;
    } else {
        if (g_FieldPadBits & FIELD_PAD_DOWN) {
            if (g_FieldPadBits & FIELD_PAD_LEFT) entity->facingAngle = 0x600;
            else if (g_FieldPadBits & FIELD_PAD_RIGHT) entity->facingAngle = 0xA00;
            else entity->facingAngle = 0x800;
        } else if (g_FieldPadBits & FIELD_PAD_UP) {
            if (g_FieldPadBits & FIELD_PAD_LEFT) entity->facingAngle = 0x200;
            else if (g_FieldPadBits & FIELD_PAD_RIGHT) entity->facingAngle = 0xE00;
            else entity->facingAngle = 0;
        } else if (g_FieldPadBits & FIELD_PAD_LEFT) entity->facingAngle = 0x400;
        else if (g_FieldPadBits & FIELD_PAD_RIGHT) entity->facingAngle = 0xC00;
        angle = entity->facingAngle + g_GeomScroll.state.position.baseAngleDigital;
    }
    if (((BattleEntity *)g_PlayerEntity) && (D_8009D2E8 & FIELD_MOVE_CONFUSED)) {
        angle += 0x800;
        {
            Combatant *core = ((BattleEntity *)g_PlayerEntity)->core;
            angle += ((unsigned int)core->stateFlags >> 7) & 0xC00;
        }
    }
    angle &= 0xFFF;
    entity->facingAngle = angle;
    /* Retail reloads both the mode flags and the stored facing angle. */
    if (D_8009D2E8 & FIELD_MOVE_CONFUSED) entity->facingAngle = (0x1000 - (u16)entity->facingAngle) & 0xFFF;
}

