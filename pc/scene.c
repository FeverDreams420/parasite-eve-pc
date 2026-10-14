#include "scene.h"
#include <stdlib.h>
#include <string.h>

void pe_scene_close(PeScene *scene) {
    pe_collision_close(&scene->collision);
    pe_room_close(&scene->room);
    free(scene->vram);
    free(scene->spu);
    free(scene->scheduler);
    free(scene->script_actors);
    memset(scene, 0, sizeof(*scene));
}

int pe_scene_prepare(PeRoomCatalog *catalog, unsigned map, PeScene *scene,
                       char *error, size_t error_size) {
    PeScene next = {0};
    if (!pe_room_load(catalog, map, &next.room)) goto catalog_error;
    if (!pe_room_collision(&next.room, &next.collision, error, error_size)) goto failed;
    next.vram = malloc(sizeof(*next.vram));
    next.spu = malloc(sizeof(*next.spu));
    if (!next.vram || !next.spu) {
        snprintf(error, error_size, "Cannot allocate scene VRAM/SPU state");
        goto failed;
    }
    if (scene->vram) memcpy(next.vram, scene->vram, sizeof(*next.vram));
    else memset(next.vram, 0, sizeof(*next.vram));
    if (scene->spu) {
        memcpy(next.spu, scene->spu, sizeof(*next.spu));
        next.sample_stats = scene->sample_stats;
    } else {
        memset(next.spu, 0, sizeof(*next.spu));
        if (!pe_spu_bank_load_startup(catalog, next.spu, &next.sample_stats)) goto catalog_error;
    }
    if (!pe_room_upload_vram(&next.room, next.vram, &next.texture_stats, error, error_size) ||
        !pe_script_program_open(&next.room, &next.scripts, error, error_size)) goto failed;
    next.script_banks = scene->script_banks;
    next.script_banks.actor_serial = 1; /* Draw_InitPipeline starts room actor IDs at one. */
    next.akao = catalog->akao;
    next.scheduler = malloc(sizeof(*next.scheduler));
    next.script_actor_count = next.scripts.actors;
    if (next.script_actor_count) next.script_actors = calloc(next.script_actor_count, sizeof(*next.script_actors));
    if (!next.scheduler || (next.script_actor_count && !next.script_actors)) {
        snprintf(error, error_size, "Cannot allocate script actors/task pool");
        goto failed;
    }
    pe_script_scheduler_init(next.scheduler, &next.scripts, &next.script_banks);
    next.scheduler->sequence = 1;
    for (unsigned i = 0; i < next.script_actor_count; ++i) {
        unsigned entry, sub_id;
        uint32_t offset;
        if (!pe_script_actor(&next.scripts, i, &entry, &sub_id) ||
            !pe_script_entry(&next.scripts, entry, &offset)) {
            snprintf(error, error_size, "Invalid initial actor script");
            goto failed;
        }
        pe_script_actor_init(&next.script_actors[i], offset);
        next.script_actors[i].field_id = next.script_banks.actor_serial++;
        next.script_actors[i].script_entry = entry;
        next.script_actors[i].sub_id = sub_id;
        if (pe_script_scheduler_start(next.scheduler, &next.script_actors[i], 2, offset) < 0) {
            snprintf(error, error_size, "%s", next.scheduler->error);
            goto failed;
        }
    }
    pe_scene_close(scene);
    *scene = next;
    /* The program borrows the room member; rebind after moving the owner. */
    scene->scripts.blob = &scene->room.data;
    scene->scheduler->program = scene->scripts;
    scene->scheduler->banks = &scene->script_banks;
    for (unsigned i = 0; i < PE_SCRIPT_TASK_CAPACITY; ++i) {
        if (!scene->scheduler->slots[i].used) continue;
        scene->scheduler->slots[i].task.program = scene->scripts;
        scene->scheduler->slots[i].task.banks = &scene->script_banks;
    }
    if (error_size) error[0] = 0;
    return 1;
catalog_error:
    snprintf(error, error_size, "%s", catalog->error);
failed:
    pe_scene_close(&next);
    return 0;
}
