#ifndef PE_PC_SCENE_H
#define PE_PC_SCENE_H

#include "script.h"
#include "vram.h"
#include "spu_bank.h"
#include "script_scheduler.h"
#include "collision.h"
#include "room_banks.h"
#include "actors.h"
#include "scene_audio.h"
#include "input.h"
#include "camera.h"

/* Native scene resources, observable without devices. Script execution,
 * actor simulation and rendering will operate on this owned state. */
typedef struct PeScene {
    PeInput input;
    PeCameraBank cameras;
    PeCamera camera;
    unsigned camera_group;
    PeCameraScroll camera_scroll;
    PeRoom room;
    PeRoomBlob entity_bank;
    PeRoomBlob area_bank;
    unsigned entity_area;
    unsigned area_entity_id,area_day;
    PeCollision collision;
    PeRoomBanks banks;
    PeVram *vram;
    PeSpuBank *spu;
    PeScriptProgram scripts;
    PeScriptBanks script_banks;
    PeAkaoGrammar akao;
    PeScriptScheduler *scheduler;
    PeActors *actors;
    PeSceneAudio audio;
    unsigned script_actor_count;
    PeVramStats texture_stats;
    PeVramStats entity_texture_stats;
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
PeScriptDispatchResult pe_scene_dispatch(void *user,PeScriptTask *task,const PeScriptCommand *command);
/* One actor's script buckets; loading context borrowed only for this call.
 * Full frame ordering/events/physics/audio synthesis remain separate. */
int pe_scene_actor_scripts(PeScene *scene,unsigned slot,PeRoomCatalog *catalog,unsigned instruction_limit);
int pe_scene_events(PeScene *scene);
void pe_scene_set_input(PeScene *scene,uint32_t held);
/* Initial primary model bank, original area IDs 10..14. Owned bytes survive
 * catalog close. Rebinding active models/animations requires later hot reload. */
int pe_scene_primary_bank(PeScene *scene,PeRoomCatalog *catalog,unsigned area,char *error,size_t error_size);
int pe_scene_area_bank(PeScene *scene,PeRoomCatalog *catalog,unsigned entity_bank,unsigned day,char *error,size_t error_size);
int pe_scene_entity_textures(PeScene *scene,PeRoomCatalog *catalog,unsigned area,unsigned day,char *error,size_t error_size);
int pe_scene_camera_select(PeScene *scene,unsigned group);
void pe_scene_camera_clamp(PeScene *scene,uint32_t x,uint32_t y);

#endif
