#include "scene.h"
#include <stdlib.h>
#include <string.h>

void pe_scene_close(PeScene *scene) {
    pe_room_close(&scene->room);
    free(scene->vram);
    free(scene->spu);
    memset(scene, 0, sizeof(*scene));
}

int pe_scene_prepare(PeRoomCatalog *catalog, unsigned map, PeScene *scene,
                       char *error, size_t error_size) {
    PeScene next = {0};
    if (!pe_room_load(catalog, map, &next.room)) goto catalog_error;
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
    pe_scene_close(scene);
    *scene = next;
    /* The program borrows the room member; rebind after moving the owner. */
    scene->scripts.blob = &scene->room.data;
    if (error_size) error[0] = 0;
    return 1;
catalog_error:
    snprintf(error, error_size, "%s", catalog->error);
failed:
    pe_scene_close(&next);
    return 0;
}
