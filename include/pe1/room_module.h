#ifndef PE1_ROOM_MODULE_H
#define PE1_ROOM_MODULE_H

#include "common.h"
#include "pe1/pm_types.h"
#include "pe1/room_interface.h"

/* A room module's class. Every effect module a room links starts with a
 * class of seven methods that the room's class table lists in slot order:
 * a no-op (RoomLib_ClassNop0), RoomLib_PlantScript, RoomLib_Spawn6, a
 * registration method (RoomLib_RegisterDrawList in most rooms), a start
 * method (RoomLib_RegisterPairedTables in most rooms), RoomLib_CloseTarget
 * and a second no-op (RoomLib_ClassNop6). The methods that are the same
 * code in every room are shared units in src/overlays/room_lib/, one per
 * function, because which of them a room links varies: some rooms replace
 * the registration or start method with their own.
 *
 * The lists the methods hand to the field engine are the module's own data,
 * named in each room's symbol file. A room that links several modules names
 * the lists of the module whose methods are the shared units. The other
 * module's methods open that module's unit in room_lib, linked in every
 * room that links the module, with the same methods and lists under the
 * module's names (orbit_set.c, drop_field.c, ground_eruption.c,
 * three_particles.c). */

struct RoomEnt;

typedef int (*RoomModuleHandler)();

/* A room's class table: one row per class, its seven methods in slot order
 * (no-op, Init, Configure, no-op, Update, Release, no-op). The rows are the
 * room's (room_lib/actor_class_table.c and its window-class variants). */
#define ROOM_CLASS_METHODS 7
extern RoomModuleHandler g_RoomClassTable[][ROOM_CLASS_METHODS];
extern PmCommand g_RoomActorClassTable[6];

/* Command program planted into the engine slot (opcode in the high half of
 * each word, -1 terminated). */
extern int g_RoomScript[];
/* Per-object handler lists, -1 terminated, one entry per object type. */
extern RoomModuleHandler g_RoomDrawList[];
extern RoomModuleHandler g_RoomUpdateList[];
extern RoomModuleHandler g_RoomInitList[];
/* Spawn layout read with g_RoomInitList (object type and placement). */
extern int g_RoomSpawnLayout[];

int RoomLib_ClassNop0(void);
int RoomLib_PlantScript(void *object);
int RoomLib_Spawn6(intptr_t a, int b, int c, intptr_t d, int e, int f);
int RoomLib_RegisterDrawList(void *o);
int RoomLib_RegisterPairedTables(void *o);
int RoomLib_CloseTarget(struct RoomEnt *o);
int RoomLib_ClassNop6(void);
/* Variants some modules link in the registration and start slots: * the draw list and start only while the engine runs the object
 * (status 3). */
int RoomLib_RegisterDrawListActive(void *o);
int RoomLib_RegisterPairedTablesActive(void *o);
int RoomLib_RegisterPairedTablesGated(void *o);

/* Arguments the scene script hands the room's effects (the script's
 * RoomLib_SetArgs3 call); three words in each room's data. */
extern int g_RoomScriptArgs[3];

intptr_t RoomLib_SetArgs3(int unused, intptr_t a, intptr_t b, intptr_t c);

/* The single argument other rooms take the same way (RoomLib_SetArg1). */
extern int g_RoomScriptArg;

int *RoomLib_SetArg1(int unused, int value);

/* The spark barrage rooms take two such words, one entry each
 * (room_lib/script_arg_pair.c). */
extern int g_RoomScriptArgA;
extern int g_RoomScriptArgB;

int *RoomLib_SetArgA(int unused, int value);
int *RoomLib_SetArgB(int unused, int value);

#endif
