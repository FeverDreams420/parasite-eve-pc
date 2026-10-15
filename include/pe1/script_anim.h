#ifndef PE1_SCRIPT_ANIM_H
#define PE1_SCRIPT_ANIM_H

#include "pe1/battle_entity_anim.h"

/* Animation setup only needs the actor's first pointer. */
typedef struct ScriptAnimObj {
    EntityAnimEventCore *core;
} ScriptAnimObj;

/* Script arguments are narrowed when stored in the six native event records. */
void Script_SetObjAnimEntry12(ScriptAnimObj *obj, int index, int effectType,
                          int enterMode, int exitMode, int power,
                          int parameter0, int parameter1, int parameter2,
                          int parameter3, int category, int frame);
void Script_SetObjAnimEntry5(ScriptAnimObj *obj, int index, int effectType,
                         int enterMode, u8 exitMode, u16 power);

void Battle_SetEntryCoords(ScriptAnimObj *obj, u8 index, int enterStep, int exitStep);

#endif
