#include "script_vm.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int main(void) {
    uint8_t bytes[256] = {0};
    PeRoomBlob blob = {0};
    blob.bytes = bytes;
    blob.size = sizeof(bytes);
    PeScriptProgram program = {0};
    program.blob = &blob;
    program.entries = 1;
    word(bytes + 8, 16);
    PeScriptBanks banks = {0};
    PeScriptTask task;
    /* Assign variable 3, set actor flag 40, wait exactly two ticks, then end. */
    word(bytes + 16, 10 | (2u << 13) | (2u << 17));
    word(bytes + 24, 3);
    word(bytes + 28, 77);
    word(bytes + 32, 65);
    word(bytes + 40, 2 | (1u << 13));
    word(bytes + 48, 2);
    word(bytes + 52, 32);
    CHECK(pe_script_task_init(&task, &program, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_WAIT);
    CHECK(banks.variables[3] == 77 && task.actor_flags == 0x40);
    CHECK(task.pc == 52 && task.active == 2 && task.instructions == 3);
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_WAIT);
    CHECK(task.pc == 52 && task.instructions == 3);
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
    CHECK(task.ticks == 3 && task.instructions == 4);
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED && task.ticks == 3);

    const struct { unsigned op; uint32_t a, b, expected; } vectors[] = {
        {0, UINT32_MAX, 1, 0}, {1, 0, 1, UINT32_MAX},
        {2, 0x10, 3, 0x13}, {3, 0x13, 0x11, 0x11}, {4, 0x13, 0x11, 2},
        {5, 2, 0, 1}, {6, 2, 0, 0}, {7, 0, 99, 1}, {8, 0, 99, UINT32_MAX},
        {9, UINT32_MAX, 0, 0}, {10, UINT32_MAX, 0, 1}, {11, 5, 5, 1},
        {12, 5, 5, 1}, {13, 6, 5, 0}, {14, 6, 5, 1},
        {15, UINT32_MAX, 2, UINT32_MAX - 1},
        {16, 0x80000000, UINT32_MAX, 0x80000000},
        {17, 1, 32, 1}, {18, 0x80000000, 31, UINT32_MAX}, {19, 42, 0, 42},
        {20, UINT32_MAX, 1, UINT32_MAX}, {20, 0x18000, 0x20000, 0x30000},
        {21, 0x10000, 0x201ff, 32512}, {21, 0xffff0000, 0x201ff, 0xffff8100},
        {22, 0xfffffff9, 3, UINT32_MAX}, {23, 0x80000000, 0, 0x80000000}
    };
    for (unsigned i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
        memset(bytes + 16, 0, sizeof(bytes) - 16);
        word(bytes + 16, 9 | (4u << 13) | (2u << 20));
        word(bytes + 24, vectors[i].op);
        word(bytes + 28, 5);
        word(bytes + 32, vectors[i].a);
        word(bytes + 36, vectors[i].b);
        word(bytes + 40, 32);
        CHECK(pe_script_task_init(&task, &program, 0, &banks));
        CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
        CHECK(banks.variables[5] == vectors[i].expected);
    }
    /* A zero divisor must not update the destination or consume the opcode. */
    word(bytes + 24, 16);
    word(bytes + 36, 0);
    banks.variables[5] = 123;
    CHECK(pe_script_task_init(&task, &program, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_ERROR);
    CHECK(banks.variables[5] == 123 && task.pc == 16 && !task.instructions);
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_ERROR && task.ticks == 1);
    /* Unknown audio command is an error, never a silent no-op. */
    word(bytes + 16, 234);
    CHECK(pe_script_task_init(&task, &program, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_ERROR);
    CHECK(strstr(task.error, "Unimplemented script opcode 234"));
    /* Infinite jump is bounded without inventing a gameplay yield. */
    word(bytes + 16, 0 | (1u << 13));
    word(bytes + 24, 0);
    CHECK(pe_script_task_init(&task, &program, 0, &banks));
    CHECK(pe_script_task_tick(&task, 7) == PE_SCRIPT_ERROR);
    CHECK(task.instructions == 7 && task.pc == 16 && strstr(task.error, "limit"));
    /* Immediate destinations write the shared original script bytes. */
    word(bytes + 16, 10 | (2u << 13));
    word(bytes + 24, 0);
    word(bytes + 28, 0x12345678);
    word(bytes + 32, 32);
    CHECK(pe_script_task_init(&task, &program, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
    CHECK(bytes[24] == 0x78 && bytes[25] == 0x56 && bytes[27] == 0x12);
    for (unsigned mode = 1; mode <= 4; ++mode) {
        word(bytes + 16, 10 | (2u << 13) | (mode << 17));
        word(bytes + 24, 0);
        word(bytes + 28, 123);
        CHECK(pe_script_task_init(&task, &program, 0, &banks));
        CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
        CHECK((mode == 1 ? task.locals[0] : mode == 2 ? banks.variables[0] :
               mode == 3 ? banks.operands[0] : banks.node_flags[0]) == 123);
    }
    puts("Script runtime: tick waits, arithmetic, shared literals, explicit unsupported commands and loop limits passed");
    return 0;
}
