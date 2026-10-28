#ifndef PE_SAVE_H
#define PE_SAVE_H

/* Mods' per-save data, kept by the host: one way for every mod.
 *
 * A mod registers each piece of per-save state once, in its init: a stable name, a layout version
 * and where the state lives in memory. From then on the host does all the work, the way the game
 * saves its own state from memory:
 *
 *   Save       the host copies every running mod's state into the card block's free tail
 *              (0x1C90..0x1FFF, inside the game's CRC) as named records.
 *   Load       the host puts each record back into its mod's memory before any mod's own
 *              PE_HOOK_SAVE_READ handler runs. A record from an older layout goes through the
 *              mod's upgrade(); a newer layout this build can't read is kept as it is, so saving
 *              doesn't destroy it, and the mod starts from its defaults.
 *   Mods off   a record whose mod is off, failed to start or isn't in this build is carried from
 *              the loaded save into every later save unchanged.
 *   Newer      a container format (or flags) newer than this build reads is kept as it is: every
 *              save writes it back unchanged and the mods start from their defaults.
 *   New Game   every record is reset and nothing carries over.
 *   Crash      pe_save_crash_tail() copies the same records from memory without calling any mod
 *              code, for a crash report: a save rebuilt from the report holds every mod's data,
 *              even when a mod caused the crash.
 *
 * Saves from builds before this (fixed blocks PELT, PECT, PESH, PKEY, SORT at their own offsets)
 * are read as version 0 records holding the old block; each owner's upgrade() reads it.
 *
 * Tail layout (little-endian, no padding):
 *   container  "PEMC", u8 format 1, u8 flags 0, u16 total bytes (header included),
 *              u32 CRC-32 (IEEE, reflected) of the whole container with this field as zero
 *   record     u32 id (FNV-1a 32 of the name's bytes), u16 layout version, u16 size, then size bytes
 * Records are sorted by id, each id once; bytes after the container are zero. */

#include <stddef.h>
#include <stdint.h>

#define PE_SAVE_TAIL_OFFSET 0x1C90
#define PE_SAVE_TAIL_SIZE   (0x2000 - PE_SAVE_TAIL_OFFSET)
#define PE_SAVE_HEADER_SIZE 12
#define PE_SAVE_RECORD_HEADER_SIZE 8

typedef struct PeSaveRecord {
    const char *name;     /* "mod.thing", e.g. "pe_qol.keys"; it is the record's id: never rename it */
    unsigned version;     /* layout of data, from 1; raise it when the layout changes */
    void *data;           /* the state itself: saved from here, loaded into here */
    unsigned size;
    /* Optional. After a load: fix up or check data; non-zero = unusable, data is reset. */
    int (*check)(void *data);
    /* Optional. Fill data from an older layout (version 0: the block of a save from before the
     * host kept mods' data); non-zero = can't, data is reset. Without it older layouts reset. */
    int (*upgrade)(void *data, const uint8_t *old, unsigned size, unsigned version);
    /* Optional. Defaults, for a save without this record and for New Game. Without it: zeros. */
    void (*reset)(void *data);
} PeSaveRecord;

/* In the mod's init (it owns the record). The record must stay valid while the game runs.
 * -1: no room left in the tail, a duplicate name or id, or bad fields (logged). */
int pe_save_register(const PeSaveRecord *record);

/* Host internals: the mod host calls these. */
void pe_save_install(void);
void pe_save_on_read(const uint8_t *block);     /* PE_HOOK_SAVE_READ, before any mod */
void pe_save_on_write(uint8_t *block);          /* PE_HOOK_SAVE_WRITE, after every mod */
void pe_save_new_game(void);
/* Crash report: the records as a save made now would hold them, into block's tail. Reads memory
 * only (no mod code, no allocation, no locks); the carried state is not changed. */
void pe_save_crash_tail(uint8_t *block);
/* Crash report: one line per record of the last crash_tail (name, owner, version, size, source). */
void pe_save_crash_describe(char *out, size_t size, size_t *used);

#endif
