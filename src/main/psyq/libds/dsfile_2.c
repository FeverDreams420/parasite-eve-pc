/* Psy-Q LIBDS DSFILE.OBJ, part 2 of 2: DS_cachefile, DS_newmedia, DS_searchdir, ds_read. */

#include "pe1/cdrom.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_ds.h"

#include "dsfile_2_externs.h"

CdlLOC *DsIntToPos(int sector, CdlLOC *position);
void *memcpy(void *, const void *, unsigned int);
int strcmp(const char *, const char *);

/* These two strings are word-aligned in dsfile.c, permitting halfword copies. */

int DS_newmedia(void) {
    u8 *cursor;
    u8 *end;
    char *name;
    int sector;
    int count;
    int read_status;
    IsoPathRecord *record;
    u8 *limit;
    char *sectors;
    char *names;

    read_status = ds_read(1, 16, g_DsFileSectorBuffer);
    if (read_status != 1) {
        if (CD_debug > 0) puts(g_DsNewMediaPvdReadErrorText);
        return 0;
    }
    if (strncmp(((IsoVolumePathTable *)g_DsFileSectorBuffer)->identifier,
                g_DsIso9660Magic, 5) != 0) {
        if (CD_debug > 0) puts(g_DsNewMediaPvdFormatErrorText);
        return 0;
    }
    end = ((IsoVolumePathTable *)g_DsFileSectorBuffer)->pathTableSectorLE;
    memcpy(&sector, end, 4);
    if (ds_read(1, sector, g_DsFileSectorBuffer) != read_status) {
        if (CD_debug > 0) printf(g_DsNewMediaPtReadErrorFormat, sector);
        return 0;
    }
    if (CD_debug > 1) puts(g_DsNewMediaSearchingDirText);
    cursor = g_DsFileSectorBuffer;
    end = cursor + 2048;
    count = 0;
    if (cursor < end) {
        sectors = (char *)&g_DslDirectoryCache[0].sector;
        names = sectors + 4;
        limit = end;
        while (1) {
            record = (IsoPathRecord *)cursor;
            if (record->nameLength == 0) break;
            memcpy(count * sizeof(DslDirectoryCacheEntry) + sectors,
                   record->sectorLE, 4);
            {
                DslNameAddress at;
                at.text = names;
                at.word = count * sizeof(DslDirectoryCacheEntry) + at.word;
                name = at.text;
            }
            g_DslDirectoryCache[count].parentDirectoryId =
                record->parentDirectoryLE[0];
            g_DslDirectoryCache[count].directoryId = count + 1;
            memcpy(name, record->name, record->nameLength);
            name[record->nameLength] = 0;
            cursor += record->nameLength + 8 + (record->nameLength & 1);
            if (CD_debug > 1)
                printf(g_DsDirectoryEntryFormat, g_DslDirectoryCache[count].sector,
                       g_DslDirectoryCache[count].directoryId,
                       g_DslDirectoryCache[count].parentDirectoryId, name);
            if (++count >= DSL_MAX_DIR) break;
            if (cursor < limit) continue;
            break;
        }
    }
finished:
    if (count < DSL_MAX_DIR) g_DslDirectoryCache[count].parentDirectoryId = 0;
    g_DsCachedDirectory = 0;
    if (CD_debug > 1) printf(g_DsDirectoryCountFormat, count);
    return 1;
}

/* Psy-Q DSFILE.OBJ: find a directory by its parent ID and name. */
int DS_searchdir(int parent, char *name) {
    int i;

    for (i = 0; i < DSL_MAX_DIR; i++) {
        if (!g_DslDirectoryCache[i].parentDirectoryId) {
            break;
        }
        if (g_DslDirectoryCache[i].parentDirectoryId != parent) {
            continue;
        }
        if (strcmp(name, g_DslDirectoryCache[i].name) == 0) {
            return i + 1;
        }
    }
    return -1;
}

/* Psy-Q DSFILE.OBJ: cache one ISO directory and its file positions. */
int DS_cachefile(int directory) {
    IsoDirectoryRecord *cursor;
    int sector;
    int count;

    if (directory == g_DsCachedDirectory) return 1;
    if (ds_read(1, (g_DslDirectoryCache + directory)[-1].sector,
                g_DsFileSectorBuffer) != 1) {
        if (CD_debug > 0) puts(g_DsCacheFileDirNotFoundText);
        return -1;
    }
    if (CD_debug > 1) puts(g_DsCacheFileSearchingText);
    cursor = (IsoDirectoryRecord *)g_DsFileSectorBuffer;
    for (count = 0; cursor < (IsoDirectoryRecord *)&g_DsFileSectorBuffer[2048];) {
        if (cursor->recordLength == 0) break;
        memcpy(&sector, cursor->sectorLE, 4);
        DsIntToPos(sector, &g_DslFileCache[count].pos);
        memcpy(&g_DslFileCache[count].size, cursor->sizeLE, 4);
        switch (count) {
        case 0:
            memcpy(g_DslFileCache[0].name, g_DsDotName, 2);
            break;
        case 1:
            memcpy(g_DslFileCache[1].name, g_DsDotDotName, 3);
            break;
        default:
            memcpy(g_DslFileCache[count].name, cursor->name, cursor->nameLength);
            g_DslFileCache[count].name[cursor->nameLength] = 0;
        }
        if (CD_debug > 1)
            printf(g_DsCacheEntryFormat, g_DslFileCache[count].pos.minute,
                   g_DslFileCache[count].pos.second,
                   g_DslFileCache[count].pos.sector,
                   g_DslFileCache[count].size, g_DslFileCache[count].name);
        cursor = ISO_NEXT_DIRECTORY_RECORD(cursor);
        if (++count >= DSL_MAX_FILE) break;
    }
    g_DsCachedDirectory = directory;
    if (count < DSL_MAX_FILE) g_DslFileCache[count].name[0] = 0;
    if (CD_debug > 1) printf(g_DsCacheCountFormat, count);
    return 1;
}

int ds_read(int count, int sector, void *destination) {
    CdlLOC position;
    int status;

    DsIntToPos(sector, &position);
    DsRead(&position, count, destination, 0x80);
    do {
        status = DsReadSync(0);
    } while (status > 0);

    return status == 0;
}
