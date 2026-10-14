#ifndef PE_PC_MODEL_H
#define PE_PC_MODEL_H
#include <stddef.h>
#include <stdint.h>
typedef struct PeModel {
    const uint8_t *bytes; size_t size;
    uint16_t primitive_blocks, vertex_count, packets[4], matrix_bytes, visible_parts;
    uint8_t part_count, animation_entries;
    uint32_t parts, vertices, colors, descriptors, section, bounds, origin, commands, textures;
} PeModel;
typedef struct PeModelTexture {
    uint8_t uv[4][2], vertices;
    uint16_t clut, page;
} PeModelTexture;
/* Checked RenderObjectHeader and contiguous data prefix, borrowing bytes.
 * Texture records are decoded separately; matrix execution/rendering pending. */
int pe_model_open(const uint8_t *bytes, size_t size, PeModel *model, char *error, size_t error_size);
/* Packet classes 0/1: quad/triangle UV, CLUT and texture page. */
int pe_model_texture(const PeModel *model, unsigned kind, unsigned index, PeModelTexture *texture);
#endif
