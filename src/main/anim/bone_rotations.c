#include "common.h"
#include "pe1/pad_state.h"
#include "pe1/render_object.h"
#include "pe1/scene_flags.h"

void Scene_CheckFlagBits(void *ctx, SceneFlagRecord **table, int *index_ptr)
{
    SceneFlagRecord *record;
    u32 flags;
    u32 mask_copy;
    int triggered;

    record = table[*index_ptr];
    triggered = 0;
    if (record != 0) {
        flags = record->flags;
    } else {
        flags = 0;
    }

    if (flags != 0) {
        do {
            void (*callback)(void *, void *);
            u32 mask;
            u32 value;

            callback = record->callback;
            if (callback == 0) {
                return;
            }

            if (flags & 0x20) {
                callback(ctx, index_ptr);
                triggered = 1;
            } else if (flags & 1) {
                value = g_FieldPadBits;
                mask = record->mask_all;
                if ((value & mask) == mask) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 2) {
                        if (value & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 2) {
                u32 current;

                current = g_FieldPadBits;
                mask = record->mask_any;
                mask_copy = mask;
                if (current & mask_copy) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & mask) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & mask_copy) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 4) {
                if (g_FieldPadPressed & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            } else if (flags & 8) {
                if (g_FieldPadReleased & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            }

            if (((flags & 0x10) == 0) && (triggered != 0)) {
                return;
            }

            record++;
            flags = record->flags;
        } while (flags != 0);
    }

    if ((record != 0) && (record->callback != 0)) {
        record->callback(ctx, index_ptr);
    }
}

/* Bone angles are decoded into scratchpad vectors indexed by matrix slot. */
#define ANIM_BONE_ANGLES 0x1F800000

/* A halfword sample base seen as the address retail adds the frame's byte
 * offset to; indexing the samples by frame hoists the scaling below the angle
 * pointer setup instead. */
typedef union AnimSampleAddress {
    u16 *samples;
    u32 address;
} AnimSampleAddress;

/* Apply the two per-object rotation overrides on top of the decoded keyframe
 * angles, then rebuild the affected bone matrices. */
void Anim_BuildRotationMatrices(RenderObjectEntity *object, RenderAnimationDataHeader *data, int frame) {
    u32 angles = ANIM_BONE_ANGLES;
    int i;

    if (data == 0) {
        return;
    }

    object->hit_body.radius = data->object_value7c;
    object->hit_body.value0 = data->object_value74;
    object->hit_body.value1 = data->object_value76;
    object->hit_body.value2 = data->object_value78;

    if ((data->encoding_flags & 3) == 2) {
        Anim_DecodeBoneRotationsShort(object, data, (s16)frame);
    } else {
        Anim_DecodeBoneRotationsByte(object, data, (s16)frame);
    }

    for (i = 0; i < 2; i++) {
        int index = object->rotation_overrides[i].matrix_index;

        if (index > 0) {
            if (object->rotation_overrides[i].flags & 0x08) {
                ((GteShortVector *)angles)[index].x = object->rotation_overrides[i].x;
            } else if (object->rotation_overrides[i].flags & 0x01) {
                ((GteShortVector *)angles)[index].x += object->rotation_overrides[i].x;
            }
            if (object->rotation_overrides[i].flags & 0x10) {
                ((GteShortVector *)angles)[index].y = object->rotation_overrides[i].y;
            } else if (object->rotation_overrides[i].flags & 0x02) {
                ((GteShortVector *)angles)[index].y += object->rotation_overrides[i].y;
            }
            if (object->rotation_overrides[i].flags & 0x20) {
                ((GteShortVector *)angles)[index].z = object->rotation_overrides[i].z;
            } else if (object->rotation_overrides[i].flags & 0x04) {
                ((GteShortVector *)angles)[index].z += object->rotation_overrides[i].z;
            }
            /* Retail ORs the slot offset into the scratchpad base here. */
            RotMatrixYXZ((GteShortVector *)(angles | (index << 3)),
                         (GteMatrix *)&object->active_matrix[index]);
        }
    }
}

/* Bone rotation keyframe decoders for the byte- and halfword-packed
 * animation formats. */

void Anim_DecodeBoneRotationsByte(RenderObjectEntity *object, RenderAnimationDataHeader *data, s16 frame) {
    GteShortVector *angle;
    s32 bone;
    s32 value;
    s32 *translation;
    s32 stride;
    s32 byteStride;
    s32 frameIndex;
    RenderMatrix *matrix;
    u8 sampleX;
    u8 sampleY;
    u8 sampleZ;
    u8 *cursor;
    s16 *angleZ;
    u8 *byteSamples;
    s16 *shortSamples;

    angle = (GteShortVector *)ANIM_BONE_ANGLES;
    cursor = (u8 *)(data + 1);
    bone = 0;
    matrix = object->matrices;
    object->active_matrix = matrix;
    translation = matrix->translation;
    stride = (data->frame_count >> 1) + 1;
    stride = stride * 4;
    do {
        shortSamples = ((RenderAnimShortChannel *)cursor)->samples.signed_values;
        if (((RenderAnimShortChannel *)cursor)->constant_marker != 0) {
            value = shortSamples[0];
            cursor += 4;
        } else {
            value = shortSamples[frame];
            cursor += stride;
        }
        *translation = value;
        bone += 1;
        translation += 1;
    } while (bone < 3);
    bone = 0;
    stride = (data->frame_count >> 2) + 1;
    if (data->last_bone_index >= bone) {
        frameIndex = frame;
        byteStride = stride * 4;
        angleZ = &angle->z;
        do {
            byteSamples = ((RenderAnimByteChannel *)cursor)->value_or_samples;
            if (((RenderAnimByteChannel *)cursor)->constant_marker != 0) {
                sampleX = byteSamples[0];
                cursor += 4;
            } else {
                sampleX = byteSamples[frameIndex];
                cursor += byteStride;
            }
            angle->x = sampleX * 0x10;
            byteSamples = ((RenderAnimByteChannel *)cursor)->value_or_samples;
            if (((RenderAnimByteChannel *)cursor)->constant_marker != 0) {
                sampleY = byteSamples[0];
                cursor += 4;
            } else {
                sampleY = byteSamples[frameIndex];
                cursor += byteStride;
            }
            angleZ[-1] = sampleY * 0x10;
            byteSamples = ((RenderAnimByteChannel *)cursor)->value_or_samples;
            /* Both branches store z, so their shared tail is cross-jumped and
             * the store stays ahead of the RotMatrixYXZ argument setup. */
            if (((RenderAnimByteChannel *)cursor)->constant_marker != 0) {
                sampleZ = byteSamples[0];
                cursor += 4;
                angleZ[0] = sampleZ * 0x10;
            } else {
                sampleZ = byteSamples[frameIndex];
                cursor += byteStride;
                angleZ[0] = sampleZ * 0x10;
            }
            RotMatrixYXZ(angle, (GteMatrix *)matrix);
            matrix++;
            angleZ += 4;
            angle++;
        } while (data->last_bone_index >= ++bone);
    }
}

void Anim_DecodeBoneRotationsShort(RenderObjectEntity *object, RenderAnimationDataHeader *data, s16 frame) {
    GteShortVector *angle;
    s16 bone;
    s32 value;
    s32 *translation;
    s32 stride;
    s32 frameOffset;
    s32 channelStride;
    RenderMatrix *matrix;
    u16 sampleX;
    u16 sampleY;
    u16 sampleZ;
    u8 *cursor;
    s16 *angleZ;
    s16 *samples;
    AnimSampleAddress sampleBase;

    angle = (GteShortVector *)ANIM_BONE_ANGLES;
    cursor = (u8 *)(data + 1);
    bone = 0;
    matrix = object->matrices;
    object->active_matrix = matrix;
    translation = matrix->translation;
    stride = (data->frame_count >> 1) + 1;
    do {
        samples = ((RenderAnimShortChannel *)cursor)->samples.signed_values;
        if (((RenderAnimShortChannel *)cursor)->constant_marker != 0) {
            value = samples[0];
            cursor += 4;
        } else {
            value = samples[frame];
            cursor += stride * 4;
        }
        *translation = value;
        translation += 1;
    } while (++bone < 3);
    bone = 0;
    if (data->last_bone_index >= bone) {
        frameOffset = frame * 2;
        channelStride = stride * 4;
        angleZ = &angle->z;
        do {
            sampleBase.samples = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values;
            if (((RenderAnimShortChannel *)cursor)->constant_marker != 0) {
                sampleX = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values[0];
                cursor = cursor + 4;
            } else {
                sampleX = *(u16 *)(frameOffset + sampleBase.address);
                cursor = cursor + channelStride;
            }
            angle->x = sampleX;
            sampleBase.samples = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values;
            if (((RenderAnimShortChannel *)cursor)->constant_marker != 0) {
                sampleY = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values[0];
                cursor = cursor + 4;
            } else {
                sampleY = *(u16 *)(frameOffset + sampleBase.address);
                cursor = cursor + channelStride;
            }
            /* Keep the vector pointer live through the Y-channel store. */
            angleZ[-1] = sampleY;
            sampleBase.samples = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values;
            if (((RenderAnimShortChannel *)cursor)->constant_marker != 0) {
                sampleZ = ((RenderAnimShortChannel *)cursor)->samples.unsigned_values[0];
                cursor = cursor + 4;
                angleZ[0] = sampleZ;
            } else {
                sampleZ = *(u16 *)(frameOffset + sampleBase.address);
                cursor = cursor + channelStride;
                angleZ[0] = sampleZ;
            }
            RotMatrixYXZ(angle, (GteMatrix *)matrix);
            matrix++;
            angleZ += 4;
            angle++;
        } while (++bone <= data->last_bone_index);
    }
}
