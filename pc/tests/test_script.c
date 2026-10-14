#include "script.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int main(void) {
    uint8_t bytes[128] = {0};
    PeRoom room = {0};
    room.map = 1;
    room.data.bytes = bytes;
    room.data.size = sizeof(bytes);
    room.data.directory[5] = (1u << 22) | 112;
    word(bytes, 96); /* relative actor map list */
    word(bytes + 4, 1);
    word(bytes + 8, 16);
    word(bytes + 116, 0); /* task record references this block */
    bytes[96] = 1;
    bytes[97] = 0;
    bytes[98] = 5;
    uint32_t header = 234 | (6u << 13) | (1u << 20) | (2u << 23) |
                      (3u << 26) | (4u << 29);
    word(bytes + 16, header);
    word(bytes + 20, 2); /* operand six's mode comes from tail */
    const uint32_t values[] = {99, 55, 511, 15, 63, 511};
    for (unsigned i = 0; i < 6; ++i) word(bytes + 24 + i * 4, values[i]);
    uint8_t original[128];
    memcpy(original, bytes, sizeof(bytes));
    PeScriptProgram program;
    char error[256];
    CHECK(pe_script_program_open(&room, &program, error, sizeof(error)));
    CHECK(!memcmp(original, bytes, sizeof(bytes))); /* no address relocation */
    unsigned entry, sub_id;
    uint32_t offset;
    CHECK(pe_script_actor(&program, 0, &entry, &sub_id));
    CHECK(entry == 0 && sub_id == 5);
    CHECK(pe_script_entry(&program, 0, &offset) && offset == 16);
    PeScriptCommand command;
    CHECK(pe_script_decode(&program, offset, &command, error, sizeof(error)));
    CHECK(command.opcode == 234 && command.count == 6 && command.next == 48);
    for (unsigned i = 0; i < 6; ++i) {
        CHECK(command.args[i].mode == (i == 5 ? 2 : i));
        CHECK(command.args[i].value == values[i]);
        CHECK(command.args[i].offset == 24 + i * 4);
    }
    CHECK(!pe_script_actor(&program, 1, &entry, &sub_id));
    CHECK(!pe_script_entry(&program, 1, &offset));
    const uint32_t bad_indices[] = {56, 512, 16, 64};
    for (unsigned i = 1; i < 5; ++i) {
        word(bytes + 24 + i * 4, bad_indices[i - 1]);
        CHECK(!pe_script_decode(&program, 16, &command, error, sizeof(error)));
        word(bytes + 24 + i * 4, values[i]);
    }
    word(bytes + 20, 7);
    CHECK(!pe_script_decode(&program, 16, &command, error, sizeof(error)));
    word(bytes + 20, 2);
    CHECK(!pe_script_decode(&program, 17, &command, error, sizeof(error)));
    CHECK(!pe_script_decode(&program, 124, &command, error, sizeof(error)));
    CHECK(!pe_script_decode(&program, UINT32_MAX, &command, error, sizeof(error)));
    word(bytes + 8, UINT32_MAX);
    CHECK(!pe_script_program_open(&room, &program, error, sizeof(error)));
    word(bytes + 8, 16);
    bytes[97] = 1;
    CHECK(!pe_script_program_open(&room, &program, error, sizeof(error)));
    bytes[97] = 0;
    word(bytes, UINT32_MAX);
    CHECK(!pe_script_program_open(&room, &program, error, sizeof(error)));
    word(bytes, 96);
    word(bytes + 4, UINT32_MAX);
    CHECK(!pe_script_program_open(&room, &program, error, sizeof(error)));
    puts("Script: self-relative entries, actor descriptors, operand modes/tail and malformed data passed");
    return 0;
}
