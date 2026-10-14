#include "model.h"
#include <stdio.h>
static uint16_t half(const uint8_t *p) { return (uint16_t)(p[0] | (unsigned)p[1]<<8); }
int pe_model_open(const uint8_t *bytes, size_t size, PeModel *model, char *error, size_t error_size) {
    if (size < 28 || size > UINT32_MAX) goto malformed;
    PeModel next = {0}; next.bytes=bytes; next.size=size;
    next.primitive_blocks=half(bytes); next.part_count=bytes[2]; next.animation_entries=bytes[3];
    next.vertex_count=half(bytes+6); next.matrix_bytes=half(bytes+24); next.visible_parts=half(bytes+26);
    unsigned primitive_count=0;
    for(unsigned i=0;i<4;++i) { next.packets[i]=half(bytes+8+i*2); primitive_count+=next.packets[i]; }
    size_t cursor=28;
    next.parts=(uint32_t)cursor; cursor+=(size_t)next.part_count*12;
    next.vertices=(uint32_t)cursor; cursor+=(size_t)next.vertex_count*8;
    next.colors=(uint32_t)cursor; cursor+=(size_t)next.vertex_count*4;
    next.descriptors=(uint32_t)cursor; cursor+=(size_t)primitive_count*12;
    next.section=(uint32_t)cursor; cursor+=16;
    next.bounds=(uint32_t)cursor; cursor+=(size_t)next.part_count*16;
    next.origin=(uint32_t)cursor; cursor+=8;
    next.commands=(uint32_t)cursor; cursor+=((unsigned)next.matrix_bytes+3u)&~3u;
    if(cursor>size) goto malformed;
    next.textures=(uint32_t)cursor;
    size_t texture_bytes=(size_t)next.packets[0]*16+(size_t)next.packets[1]*12;
    if(texture_bytes>size-cursor) goto malformed;
    for(unsigned i=0;i<next.part_count;++i) {
        const uint8_t *part=bytes+next.parts+i*12;
        if ((unsigned)half(part)+half(part+2)>next.vertex_count) goto malformed;
    }
    *model=next; if(error_size) error[0]=0; return 1;
malformed:
    snprintf(error,error_size,"Malformed render model prefix"); return 0;
}

int pe_model_texture(const PeModel *model, unsigned kind, unsigned index, PeModelTexture *texture) {
    if(kind>1 || index>=model->packets[kind]) return 0;
    size_t offset=model->textures;
    if(kind) offset+=(size_t)model->packets[0]*16;
    offset+=(size_t)index*(kind ? 12 : 16);
    if(offset>model->size || (kind ? 12u : 16u)>model->size-offset) return 0;
    const uint8_t *record=model->bytes+offset;
    PeModelTexture next={0}; next.vertices=kind ? 3 : 4;
    for(unsigned i=0;i<next.vertices;++i) { next.uv[i][0]=record[i*4]; next.uv[i][1]=record[i*4+1]; }
    next.clut=half(record+2); next.page=half(record+6); *texture=next; return 1;
}
