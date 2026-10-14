#include "host_disc.h"
#include "pe1/cdrom.h"
#include "pe1/game_state.h"
#include <stdint.h>

static PeDisc *mounted_disc;
static PeDiscFile mounted_archive;
static int completion;
void pe_host_disc_bind(PeDisc *disc, const PeDiscFile *archive)
{
    mounted_disc = archive ? disc : NULL;
    if (archive) mounted_archive = *archive;
    completion = 0;
    g_GameState.flags &= ~0x1004000u;
}

/* Host I/O replaces drive negotiation and queues. Data is read immediately;
 * the original completion poll still releases the game's read-in-progress flags. */
int CdRom_ReadSectorsFromLba(u32 lba, void *destination, u32 sectors)
{
    if (g_GameState.flags & 0x1000000u) return -1;
    completion = -1;
    if (!mounted_disc || !destination || lba < mounted_archive.sector) return -1;
    uint64_t offset = (uint64_t)(lba - mounted_archive.sector) * 2048u;
    uint64_t bytes = (uint64_t)sectors * 2048u;
    if (offset > mounted_archive.size || bytes > mounted_archive.size - offset || bytes > SIZE_MAX) return -1;
    if (!pe_disc_read(mounted_disc, &mounted_archive, offset, destination, (size_t)bytes)) return -1;
    g_GameState.flags |= 0x1004000u;
    completion = 0;
    return 1;
}

int CdRom_PollReady(void)
{
    g_GameState.flags &= ~0x1004000u;
    return completion;
}
