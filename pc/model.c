#include "model.h"
#include <stdio.h>
static uint16_t half(const uint8_t *p) { return (uint16_t)(p[0] | (unsigned)p[1]<<8); }
static uint32_t word(const uint8_t *p) { return half(p) | (uint32_t)half(p+2)<<16; }
static int16_t signed_half(uint16_t v) { return (int16_t)(v&32768 ? (int)v-65536 : (int)v); }
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
    unsigned descriptor=0;
    for(unsigned kind=0;kind<4;++kind) for(unsigned i=0;i<next.packets[kind];++i,++descriptor) {
        const uint8_t *record=bytes+next.descriptors+descriptor*12;
        for(unsigned v=0;v<(kind&1 ? 3u : 4u);++v)
            if(half(record+4+v*2)>=next.vertex_count) goto malformed;
    }
    *model=next; if(error_size) error[0]=0; return 1;
malformed:
    snprintf(error,error_size,"Malformed render model prefix"); return 0;
}

static const uint8_t *record_at(const PeModel *model,uint64_t offset,unsigned length) {
    if(offset>model->size || length>model->size-offset) return NULL;
    return model->bytes+(size_t)offset;
}
int pe_model_vertex(const PeModel *model,unsigned index,PeModelVertex *vertex) {
    if(index>=model->vertex_count) return 0;
    const uint8_t *record=record_at(model,(uint64_t)model->vertices+index*8u,8);
    const uint8_t *color=record_at(model,(uint64_t)model->colors+index*4u,4);
    if(!record || !color) return 0;
    *vertex=(PeModelVertex){signed_half(half(record)),signed_half(half(record+2)),signed_half(half(record+4)),half(record+6),word(color)};
    return 1;
}
int pe_model_part(const PeModel *model,unsigned index,PeModelPart *part) {
    if(index>=model->part_count) return 0;
    const uint8_t *record=record_at(model,(uint64_t)model->parts+index*12u,12);
    if(!record) return 0;
    if((unsigned)half(record)+half(record+2)>model->vertex_count) return 0;
    *part=(PeModelPart){half(record),half(record+2),record[4],word(record+8)}; return 1;
}
int pe_model_primitive(const PeModel *model,unsigned kind,unsigned index,PeModelPrimitive *primitive) {
    if(kind>3 || index>=model->packets[kind]) return 0;
    unsigned preceding=0; for(unsigned i=0;i<kind;++i) preceding+=model->packets[i];
    const uint8_t *record=record_at(model,(uint64_t)model->descriptors+(preceding+index)*12u,12);
    if(!record) return 0;
    PeModelPrimitive next={0}; next.kind=record[3]; next.vertices=kind&1 ? 3 : 4;
    for(unsigned v=0;v<next.vertices;++v) {
        next.vertex[v]=half(record+4+v*2); if(next.vertex[v]>=model->vertex_count) return 0;
    }
    *primitive=next; return 1;
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
