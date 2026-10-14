#ifndef PE_PC_SCRIPT_H
#define PE_PC_SCRIPT_H

#include "room.h"

typedef struct PeScriptProgram {
    const PeRoomBlob *blob; /* borrowed, immutable archive */
    uint32_t block, map_list;
    unsigned entries, actors;
} PeScriptProgram;

typedef struct PeScriptArgument {
    uint32_t value;
    uint32_t offset; /* literal location for a future writable script copy */
    unsigned mode; /* 0 literal, 1 actor, 2 variables, 3 operands, 4 node flags */
} PeScriptArgument;

typedef struct PeScriptCommand {
    unsigned opcode, count;
    uint32_t offset, next;
    PeScriptArgument args[15];
} PeScriptCommand;

int pe_script_program_open(const PeRoom *room, PeScriptProgram *program,
                            char *error, size_t error_size);
int pe_script_entry(const PeScriptProgram *program, unsigned entry, uint32_t *offset);
/* Original Scene_LoadMap receives a two-byte actor descriptor: script ID,
 * entity sub-ID. The room's map list supplies these pairs in spawn order. */
int pe_script_actor(const PeScriptProgram *program, unsigned actor,
                     unsigned *entry, unsigned *sub_id);
int pe_script_decode(const PeScriptProgram *program, uint32_t offset,
                      PeScriptCommand *command, char *error, size_t error_size);

#endif
