#include "pe1/render_object.h"
#include <stdio.h>
#include <string.h>

void Anim_BuildRotationMatrices(RenderObjectEntity *, RenderAnimationDataHeader *, int);

static int check(int short_encoding)
{
    struct {
        RenderAnimationDataHeader header;
        u16 channels[20];
    } data = {0};
    RenderObjectEntity object = {0};
    RenderMatrix matrices[2] = {0};
    int i;
    data.header.encoding_flags = short_encoding ? 2 : 0;
    data.header.last_bone_index = 1;
    data.header.frame_count = 2;
    data.header.object_value74 = 11;
    data.header.object_value76 = 22;
    data.header.object_value78 = 33;
    data.header.object_value7c = 44;
    for (i = 0; i < 9; ++i) data.channels[i*2] = 1;
    data.channels[1] = (u16)-100;
    data.channels[3] = 200;
    data.channels[5] = 300;
    /* Nine constant channels: three translations and two XYZ rotations. */
    if (short_encoding) data.channels[9] = 1024;
    else ((u8 *)data.channels)[17] = 64;
    object.matrices = matrices;
    object.rotation_overrides[0].matrix_index = 1;
    object.rotation_overrides[0].flags = 0x10;
    object.rotation_overrides[0].y = 1024;
    if (short_encoding == 2) {
        /* A sampled halfword Y channel selects frame 1 at a host address. */
        data.channels[14] = 0;
        data.channels[15] = 0;
        data.channels[16] = 1024;
        data.channels[18] = 1;
        object.rotation_overrides[0].flags = 0;
    }
    Anim_BuildRotationMatrices(&object, &data.header, 1);
    if (object.active_matrix != matrices || matrices[0].translation[0] != -100 ||
        matrices[0].translation[1] != 200 || matrices[0].translation[2] != 300 ||
        object.hit_body.radius != 44 || object.hit_body.value0 != 11 ||
        object.hit_body.value1 != 22 || object.hit_body.value2 != 33) return 1;
    for (i = 0; i < 2; ++i) {
        if (matrices[i].rotation[0][0] != 0 || matrices[i].rotation[0][2] != 4096 ||
            matrices[i].rotation[1][1] != 4096 || matrices[i].rotation[2][0] != -4096)
            return 2;
    }
    return 0;
}

int main(void)
{
    int error = check(0);
    if (error) return error;
    error = check(1);
    if (error) return error;
    error = check(2);
    if (error) return error;
    puts("Original byte/halfword bone decoding and rotation overrides passed");
    return 0;
}
