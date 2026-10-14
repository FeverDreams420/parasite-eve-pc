#include "script.h"
#include <string.h>

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

int pe_script_entry(const PeScriptProgram *program, unsigned entry, uint32_t *offset) {
    if (entry >= program->entries) return 0;
    uint32_t relative = read32(program->blob->bytes + program->block + 8 + entry * 4);
    uint64_t absolute = (uint64_t)program->block + relative;
    if (absolute > program->blob->size || program->blob->size - absolute < 8 ||
        (absolute & 1)) return 0;
    *offset = (uint32_t)absolute;
    return 1;
}

int pe_script_actor(const PeScriptProgram *program, unsigned actor,
                     unsigned *entry, unsigned *sub_id) {
    if (actor >= program->actors) return 0;
    const uint8_t *pair = program->blob->bytes + program->map_list + 1 + actor * 2;
    if (pair[0] >= program->entries) return 0;
    *entry = pair[0];
    *sub_id = pair[1];
    return 1;
}

int pe_script_program_open(const PeRoom *room, PeScriptProgram *program,
                            char *error, size_t error_size) {
    PeScriptProgram next = {0};
    next.blob = &room->data;
    const uint8_t *records;
    unsigned count;
    if (!pe_room_blob_table(next.blob, 5, 8, &records, &count) || !count) goto malformed;
    next.block = read32(records + 4) & UINT32_C(0xffffff);
    size_t size = next.blob->size;
    if (size > UINT32_MAX || next.block > size || size - next.block < 8) goto malformed;
    const uint8_t *block = next.blob->bytes + next.block;
    next.entries = read32(block + 4);
    if (next.entries > (size - next.block - 8) / 4) goto malformed;
    /* Retain self-relative data. Never mutate 32-bit offsets into truncated
     * host pointers as Script_RelocBlock did on PS1. */
    uint64_t map_list = (uint64_t)next.block + read32(block);
    if (map_list >= size) goto malformed;
    next.map_list = (uint32_t)map_list;
    next.actors = next.blob->bytes[next.map_list];
    if (next.actors > (size - next.map_list - 1) / 2) goto malformed;
    for (unsigned i = 0; i < next.entries; ++i) {
        uint32_t offset;
        if (!pe_script_entry(&next, i, &offset)) goto malformed;
    }
    for (unsigned i = 0; i < next.actors; ++i) {
        unsigned entry, sub_id;
        if (!pe_script_actor(&next, i, &entry, &sub_id)) goto malformed;
    }
    *program = next;
    if (error_size) error[0] = 0;
    return 1;
malformed:
    snprintf(error, error_size, "Map %u: malformed self-relative script block", room->map);
    return 0;
}

int pe_script_decode(const PeScriptProgram *program, uint32_t offset,
                      PeScriptCommand *command, char *error, size_t error_size) {
    const PeRoomBlob *blob = program->blob;
    if (blob->size > UINT32_MAX || offset > blob->size ||
        blob->size - offset < 8 || (offset & 1)) goto truncated;
    const uint8_t *bytes = blob->bytes + offset;
    uint32_t header = read32(bytes), modes = header >> 17, tail = read32(bytes + 4);
    PeScriptCommand next = {0};
    next.opcode = header & 8191;
    next.count = (header >> 13) & 15;
    next.offset = offset;
    if (next.count > (blob->size - offset - 8) / 4) goto truncated;
    next.next = offset + 8 + next.count * 4;
    for (unsigned i = 0; i < next.count; ++i) {
        PeScriptArgument *arg = &next.args[i];
        arg->mode = modes & 7;
        arg->offset = offset + 8 + i * 4;
        arg->value = read32(blob->bytes + arg->offset);
        /* These bank sizes follow Script_RunQueue and pool initialization.
         * Actor-local words occupy FieldActor.pad_0AC[0x38]. */
        const unsigned limits[] = {0, 0x38, 0x200, 16, 64};
        if (arg->mode > 4 || (arg->mode && arg->value >= limits[arg->mode])) {
            snprintf(error, error_size, "Script at 0x%x: invalid operand %u mode %u value %u",
                     offset, i, arg->mode, arg->value);
            return 0;
        }
        modes >>= 3;
        if (i == 4) modes = tail;
    }
    *command = next;
    if (error_size) error[0] = 0;
    return 1;
truncated:
    snprintf(error, error_size, "Truncated or misaligned script command at 0x%x", offset);
    return 0;
}
