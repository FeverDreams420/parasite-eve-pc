#ifndef PE_PC_MODEL_H
#define PE_PC_MODEL_H
#include <stddef.h>
#include <stdint.h>
typedef struct PeModel {
    const uint8_t *bytes; size_t size;
    uint16_t primitive_blocks, vertex_count, packets[4], matrix_bytes, visible_parts;
    uint8_t part_count, animation_entries;
    uint32_t parts, vertices, colors, descriptors, section, bounds, origin, commands, textures;
    uint16_t matrix_outputs, max_stack_depth;
} PeModel;
typedef struct PeModelTexture {
    uint8_t uv[4][2], vertices;
    uint16_t clut, page;
} PeModelTexture;
typedef struct PeModelVertex { int16_t x,y,z; uint16_t normal; uint32_t color; } PeModelVertex;
typedef struct PeModelPart { uint16_t first,count; uint8_t visible; uint32_t translation_z; } PeModelPart;
typedef struct PeModelPrimitive { uint16_t vertex[4]; uint8_t vertices,kind; } PeModelPrimitive;
typedef enum PeMatrixCommandKind { PE_MATRIX_PART, PE_MATRIX_PUSH, PE_MATRIX_POP } PeMatrixCommandKind;
typedef struct PeMatrixCommand { PeMatrixCommandKind kind; uint8_t part; } PeMatrixCommand;
typedef struct PeModelBounds { int16_t lower[3],upper[3],radius,attributes; } PeModelBounds;
/* Checked RenderObjectHeader and contiguous data prefix, borrowing bytes.
 * Texture records are decoded separately; matrix execution/rendering pending. */
int pe_model_open(const uint8_t *bytes, size_t size, PeModel *model, char *error, size_t error_size);
/* Packet classes 0/1: quad/triangle UV, CLUT and texture page. */
int pe_model_texture(const PeModel *model, unsigned kind, unsigned index, PeModelTexture *texture);
int pe_model_vertex(const PeModel *model,unsigned index,PeModelVertex *vertex);
int pe_model_part(const PeModel *model,unsigned index,PeModelPart *part);
int pe_model_primitive(const PeModel *model,unsigned kind,unsigned index,PeModelPrimitive *primitive);
int pe_model_matrix_command(const PeModel *model,unsigned index,PeMatrixCommand *command);
int pe_model_bounds(const PeModel *model,unsigned part,PeModelBounds *bounds);
#endif
