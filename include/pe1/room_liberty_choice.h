#ifndef PE1_ROOM_LIBERTY_CHOICE_H
#define PE1_ROOM_LIBERTY_CHOICE_H

#include "common.h"

/* Selects a weighted outcome using the two input pointers in the script
 * parameter block: variant bits and distance band. */
typedef struct RoomLibertyChoiceInputs {
    s32 *variantBits;
    s32 *distance;
} RoomLibertyChoiceInputs;

PE1_STATIC_ASSERT(sizeof(RoomLibertyChoiceInputs) == 8, room_liberty_choice_inputs_size);

s32 RoomM273_ChooseWeightedAction(s32 value, RoomLibertyChoiceInputs *inputs);

#endif
