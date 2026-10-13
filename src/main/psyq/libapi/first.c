/* Psy-Q LIBAPI FIRST.OBJ: firstfile, Sys_FirstFileHookCallback. */

#include "pe1/bios_firstfile.h"

/* SDK firstfile wrapper (FIRST.OBJ). Constraints are tracked in crutch debt. */
int strcmp(const char *, const char *);
void *firstfile2(const char *, void *);
int Sys_FirstFileHookCallback(int *, unsigned, unsigned);
void *firstfile(const char *inName, void *inResult) {
    const char *name = inName;
    void *result = inResult;
    const signed char *input;
    char *out;
    BiosDeviceEntry *entry;
    BiosDeviceEntry *end;
    unsigned count;
    int found;
    out = g_FirstFileDeviceName;
    if (*(const signed char *)name >= 59) {
        input = (const signed char *)name;
        do {
            *out++ = *input++;
        } while (*input >= 59);
    }
    *out = 0;
    count = BIOS_DEVICE_DIRECTORY->byteLength;
    entry = BIOS_DEVICE_DIRECTORY->entries;
    count /= sizeof(BiosDeviceEntry);
    end = entry + count;
    if (entry < end) {
        BiosDeviceEntry *limit = end;
        do {
            if (entry->name && strcmp(entry->name, g_FirstFileDeviceName) == 0) {
                g_FirstFileOriginalHandler = entry->firstFile;
                found = 1;
                goto check;
            }
            entry++;
        } while (entry < limit);
    }
    found = 0;
check:
    if (!(found)) {
    return 0;
    }
second:
    count = BIOS_DEVICE_DIRECTORY->byteLength;
    entry = BIOS_DEVICE_DIRECTORY->entries;
    count /= sizeof(BiosDeviceEntry);
    end = entry + count;
    if (entry < end) {
        BiosDeviceEntry *limit = end;
        do {
            if (entry->name && strcmp(entry->name, g_FirstFileDeviceName) == 0) {
                entry->firstFile = Sys_FirstFileHookCallback;
                break;
            }
            entry++;
        } while (entry < limit);
    }
call:
    return firstfile2(name, result);
}

/* Restore the first matching device's handler, then forward this request.
 * The handler used for restoration is saved before the name comparisons;
 * the callback used for forwarding is read again afterward. The empty
 * barriers are included in debt. */
int Sys_FirstFileHookCallback(int *file, unsigned int arg1, unsigned int arg2) {
    BiosDeviceEntry *entry;
    BiosDeviceEntry *limit;
    BiosFirstFileHandler original;
    unsigned int count;
    int *fileArg;

    if (!*file) *file = 1;
    count = BIOS_DEVICE_DIRECTORY->byteLength;
    entry = BIOS_DEVICE_DIRECTORY->entries;
    original = g_FirstFileOriginalHandler;
    count /= sizeof(BiosDeviceEntry);
    limit = entry + count;
    fileArg = file;
    if (entry < limit) {
        BiosDeviceEntry *end = limit;
        do {
            if (entry->name && strcmp(entry->name, g_FirstFileDeviceName) == 0) {
                entry->firstFile = original;
                break;
            }
            entry++;
        } while (entry < end);
        fileArg = file;
    }
    return g_FirstFileOriginalHandler(fileArg, arg1, arg2);
}
