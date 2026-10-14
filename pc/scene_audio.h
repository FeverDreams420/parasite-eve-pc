#ifndef PE_PC_SCENE_AUDIO_H
#define PE_PC_SCENE_AUDIO_H
#include "music.h"
#include "script_vm.h"
typedef struct PeSceneAudio {
    PeMusic *music[2]; /* owned stable objects; caller closes them */
    PeRoomCatalog *catalog; /* borrowed loading context */
    const PeRoom *room;
    const PeSpuBank *resident;
    PeAkaoControl initial;
    int keys[2][2], banks[2];
    unsigned volumes[2];
    uint32_t flags;
    uint32_t *game_flags; /* optional shared g_GameState.flags; otherwise flags */
} PeSceneAudio;
void pe_scene_audio_reset(PeSceneAudio *audio);
/* Decoded actor opcode 234. Unsupported commands preserve audio state. */
PeScriptDispatchResult pe_scene_audio_dispatch(void *user, PeScriptTask *task,
                                               const PeScriptCommand *command);
#endif
