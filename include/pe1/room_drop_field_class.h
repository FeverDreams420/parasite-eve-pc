#ifndef PE1_ROOM_DROP_FIELD_CLASS_H
#define PE1_ROOM_DROP_FIELD_CLASS_H

#include "common.h"
#include "pe1/room_interface.h"
#include "pe1/room_module.h"

/* The drop field effect set's module class (drop_field.c), in
 * class-table slot order, and the script and lists it hands the field
 * engine, named in each room's symbol file. The start method spawns from
 * the spawn data with the spawn script as the layout; the set's effects
 * (pe1/room_drop_field.h) spawn from the same two tables. */
extern int g_RoomDropFieldScript[48];
extern FieldFxHandler g_RoomDropFieldDrawList[14], g_RoomDropFieldUpdateList[14], g_RoomDropFieldSpawnData[14];
extern u16 g_RoomDropFieldSpawnScript[14];
extern PmCommand g_RoomDropFieldHandlers;

void RoomEffect_DropFieldNop0(void);
void RoomEffect_DropFieldPlantScript(PmSlotHeader *object);
int RoomEffect_DropFieldSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f);
int RoomEffect_DropFieldRegister(PmSlotHeader *slot);
int RoomEffect_DropFieldStart(PmSlotHeader *slot);
int RoomEffect_DropFieldClose(PmSlotHeader *slot);
void RoomEffect_DropFieldNop6(void);

#endif
