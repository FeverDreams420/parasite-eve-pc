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
/* Checked RenderObjectHeader and contiguous data prefix, borrowing bytes.
 * Matrix command execution, texture records and rendering are separate work. */
int pe_model_open(const uint8_t *bytes, size_t size, PeModel *model, char *error, size_t error_size);
#endif
