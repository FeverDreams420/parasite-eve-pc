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
    if (opcode == 0xc8) {
        PeSceneAudio *audio = user;
        if (command->count < 6 || !audio->catalog || !audio->room || !audio->resident)
            return PE_DISPATCH_ERROR;
        unsigned key = pe_script_arg_read(task, &command->args[1]);
        for (unsigned slot = 0; slot < 2; ++slot) {
            if (audio->keys[slot][0] != (int)key) continue;
            if (audio->flags & (0x40u << slot)) return PE_DISPATCH_CONTINUE;
            /* Cached/secondary-bank restarting requires its own implementation. */
            return PE_DISPATCH_UNSUPPORTED;
        }
        const uint8_t *records; unsigned count, index;
        if (!pe_room_blob_table(&audio->room->data, 12, 12, &records, &count)) return PE_DISPATCH_ERROR;
        for (index = 0; index < count; ++index)
            if ((unsigned)(records[index * 12 + 10] | (unsigned)records[index * 12 + 11] << 8) == key) break;
        if (index == count) return PE_DISPATCH_CONTINUE; /* Original missing-key no-op. */
        char error[256];
        PeRoomMusic record;
        if (!pe_room_music(audio->room, index, &record, error, sizeof(error))) return PE_DISPATCH_ERROR;
        if (audio->music[0] && audio->music[0]->player.bank_id == record.sequence.id)
            return PE_DISPATCH_UNSUPPORTED; /* Same-ID F0/queue ordering is pending. */
        if (!pe_music_open(audio->catalog, audio->room, (uint16_t)key, audio->resident,
                           &audio->initial, &audio->music[0], error, sizeof(error))) {
            snprintf(task->error, sizeof(task->error), "%s", error);
            return PE_DISPATCH_ERROR;
        }
        PeAkaoPlayer *player = &audio->music[0]->player;
        player->control.volume = UINT32_C(0x7f0000);
        player->control.volume_duration = 0;
        player->control.volume_dirty = 1;
        for (unsigned voice = 0; voice < 24; ++voice)
            if (player->active & (UINT32_C(1) << voice)) player->channels[voice].update_flags |= 3;
        audio->keys[0][0] = (int)key;
        audio->keys[0][1] = player->bank_id;
        audio->banks[0] = audio->music[0]->bank;
        audio->volumes[0] = 0x7f;
        audio->flags |= 0x40;
        pe_script_arg_write(task, &command->args[5], player->bank_id);
        return PE_DISPATCH_CONTINUE;
    }
    if (opcode != 0xd9) return PE_DISPATCH_UNSUPPORTED;
    pe_scene_audio_reset(user);
    return PE_DISPATCH_CONTINUE;
}
