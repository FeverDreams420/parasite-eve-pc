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
extern int g_RoomDropFieldScript[];
extern RoomModuleHandler g_RoomDropFieldDrawList[];
extern RoomModuleHandler g_RoomDropFieldUpdateList[];
extern u8 g_RoomDropFieldSpawnScript[];
extern u8 g_RoomDropFieldSpawnData[];

int RoomEffect_DropFieldNop0(void);
int RoomEffect_DropFieldPlantScript(void *object);
int RoomEffect_DropFieldSpawn6(intptr_t a, int b, int c, int d, int e, int f);
int RoomEffect_DropFieldRegister(struct RoomEnt *o);
int RoomEffect_DropFieldStart(void *o);
int RoomEffect_DropFieldClose(struct RoomEnt *o);
int RoomEffect_DropFieldNop6(void);

#endif
