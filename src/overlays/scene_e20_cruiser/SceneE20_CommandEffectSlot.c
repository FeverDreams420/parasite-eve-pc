#include "pe1/random.h"
#include "pe1/scene_e20_hover_orb.h"

intptr_t SceneE20_CommandEffectSlot(int command, intptr_t index, intptr_t value, intptr_t z) {
    if ((uintptr_t)index >= 3) return 0;
    SceneEffectSlot *slot = &g_SceneE20EffectSlots[index];
    int random;

    slot->pending = 1;
    switch (command) {
    case 1:
        slot->target.x = value;
        random = rand();
        slot->target.y = g_RoomFloorY->y - 600;
        slot->target.y -= random & 0x1FF;
        slot->target.z = z;
        slot->pending = 0;
        break;
    case 2:
        slot->position.x = value;
        slot->position.y = g_RoomFloorY->y;
        slot->position.z = z;
        break;
    case 3:
        slot->target.x = value;
        random = rand();
        slot->target.y = g_RoomFloorY->y - 600;
        slot->target.y -= random & 0x1FF;
        slot->target.z = z;
        break;
    case 4:
        slot->duration = value;
        break;
    }
    return (intptr_t)slot;
}
