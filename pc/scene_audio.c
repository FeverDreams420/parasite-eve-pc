#include "scene_audio.h"

void pe_scene_audio_reset(PeSceneAudio *audio) {
    for (unsigned slot = 0; slot < 2; ++slot) {
        audio->keys[slot][0] = audio->keys[slot][1] = -1;
        audio->banks[slot] = -1;
        if (audio->music[slot]) pe_akao_player_release(&audio->music[slot]->player);
    }
    audio->flags &= ~UINT32_C(0xf0);
}

PeScriptDispatchResult pe_scene_audio_dispatch(void *user, PeScriptTask *task,
                                               const PeScriptCommand *command) {
    if (command->opcode != 234 || !command->count) return PE_DISPATCH_UNSUPPORTED;
    unsigned opcode = pe_script_arg_read(task, &command->args[0]);
    if (opcode != 0xd9) return PE_DISPATCH_UNSUPPORTED;
    pe_scene_audio_reset(user);
    return PE_DISPATCH_CONTINUE;
}
