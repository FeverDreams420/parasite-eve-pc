#include "pe1/render_object.h"
#include <stdio.h>
#include <string.h>

static int checks(void)
{
    RenderObjectHeader header = {0};
    RenderObjectPart parts[3] = {0};
    RenderVec3s bounds[6] = {0};
    RenderAnimationLookupEntry centres[3] = {0};
    RenderMatrix source[3] = {0}, output[3] = {0};
    RenderObjectEntity object = {0}, owner = {0};
    s8 commands[] = {0, -1, 1, -2, 2};
    int i;
    for (i = 0; i < 3; ++i) {
        source[i].rotation[0][0] = source[i].rotation[1][1] = source[i].rotation[2][2] = 4096;
        parts[i].visible = 1;
        bounds[i*2].x = 10;
        bounds[i*2+1].pad = 0;
    }
    source[0].translation[0] = 100;
    parts[1].translation_z = 200;
    parts[2].translation_z = -300;
    object.model_matrix = source[2];
    object.model_matrix.translation[1] = 50;
    header.matrix_command_bytes = sizeof(commands);
    object.header = &header;
    object.parts = parts;
    object.bounds_vertices = bounds;
    object.matrix_commands = commands;
    object.active_matrix = source;
    object.matrices = output;
    object.animation_entries = centres;
    Render_TransformVertices(&object);
    if (output[0].translation[0] != 100 || output[0].translation[1] != 50 ||
        output[1].translation[0] != 100 || output[1].translation[2] != 200 ||
        output[2].translation[0] != 100 || output[2].translation[2] != -300)
        return 1;
    if (centres[0].value0 != 110 || centres[1].value2 != 200 ||
        centres[2].value2 != -300) return 2;
    owner.matrices = source;
    source[1].translation[0] = 999;
    object.animation_source = &owner;
    object.animation_id = 1;
    object.animation_state = 3;
    object.flags_9C = 0;
    Render_TransformVertices(&object);
    if (output[0].translation[0] != 999 || output[1].translation[0] != 999) return 3;
    object.flags_9C = 0x400;
    object.model_matrix.translation[0] = 777;
    Render_TransformVertices(&object);
    if (output[0].translation[0] != 999 || output[1].translation[0] != 777) return 4;
    object.animation_state = 1;
    object.flags_9C = 0;
    Render_TransformVertices(&object);
    if (object.model_matrix.translation[0] != 999 ||
        output[0].translation[0] != 1099 || output[2].translation[2] != -300)
        return 5;
    object.animation_state = 4;
    object.model_matrix.translation[0] = 777;
    Render_TransformVertices(&object);
    if (object.model_matrix.translation[0] != 1776 ||
        output[0].translation[0] != 1876) return 6;
    return 0;
}
int main(void)
{
    int error = checks();
    if (error) { fprintf(stderr,"original transform check %d failed\n",error); return error; }
    puts("Original matrix command stack, hit centres and inherited copies passed");
    return 0;
}
