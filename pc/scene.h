#ifndef PE_PC_SCENE_H
#define PE_PC_SCENE_H

#include "script.h"
#include "vram.h"
#include "spu_bank.h"
#include "script_scheduler.h"
#include "collision.h"
#include "room_banks.h"

/* Native scene resources, observable without devices. Script execution,
 * actor simulation and rendering will operate on this owned state. */
typedef struct PeScene {
    PeRoom room;
    PeCollision collision;
    PeRoomBanks banks;
    PeVram *vram;
    PeSpuBank *spu;
    PeScriptProgram scripts;
    PeScriptBanks script_banks;
    PeAkaoGrammar akao;
    PeScriptScheduler *scheduler;
    PeScriptActor *script_actors;
    unsigned script_actor_count;
    PeVramStats texture_stats;
    PeSpuBankStats sample_stats;
} PeScene;

/* Start zero-initialized. Prepare is transactional; failed scene changes
 * preserve the old scene; changes retain existing VRAM/SPU contents before
 * room uploads. The startup sample bank is loaded only for a fresh state.
 * The catalog source only has to stay open while
 * preparation is running. */
int pe_scene_prepare(PeRoomCatalog *catalog, unsigned map, PeScene *scene,
                       char *error, size_t error_size);
void pe_scene_close(PeScene *scene);

#endif
