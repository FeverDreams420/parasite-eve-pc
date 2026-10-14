#ifndef PE_PC_MATRIX_H
#define PE_PC_MATRIX_H
#include "model.h"
typedef struct PeMatrix { int16_t rotation[3][3]; uint32_t translation[3]; } PeMatrix;
void pe_matrix_identity(PeMatrix *matrix);
/* Signed 16-bit angles, 4096 units per turn. Preserves translation. */
void pe_matrix_rotation_yxz(const uint16_t angles[3],PeMatrix *matrix);
/* RTIR/RT composition used by model commands: 12 fractional rotation bits,
 * saturated signed-halfword rotation, signed-halfword source translation,
 * wrapped full-word output translation. Inputs may alias output. */
void pe_matrix_compose(const PeMatrix *parent,const PeMatrix *source,PeMatrix *output);
/* RT point result: full MAC words and saturated IR halfwords, sf=1/lm=0. */
typedef struct PeTransformedPoint { uint32_t word[3]; int16_t saturated[3]; } PeTransformedPoint;
void pe_matrix_point(const PeMatrix *matrix,const int16_t point[3],PeTransformedPoint *output);
/* Core command-mode pose. Root uses source translation; other parts use
 * (0,0,part.translation_z). Failure preserves output/written. Animation
 * modes, hit-volume transforms and view projection are separate work. */
int pe_model_matrices(const PeModel *model,const PeMatrix *base,const PeMatrix *sources,
                       unsigned source_count,PeMatrix *output,unsigned capacity,unsigned *written);
typedef struct PeHitVolume { int16_t center[3]; uint16_t part; int16_t radius,attributes; } PeHitVolume;
/* Command-mode centres in original command order; metadata in part order.
 * Requires a complete set of eligible centres. Failure preserves output. */
int pe_model_hitvolumes(const PeModel *model,const PeMatrix *pose,unsigned pose_count,
                         PeHitVolume *output,unsigned capacity,unsigned *written);
#endif
