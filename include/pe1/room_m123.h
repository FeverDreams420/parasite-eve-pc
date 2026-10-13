#ifndef PE1_ROOM_M123_H
#define PE1_ROOM_M123_H

#include "common.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_m123_burst_particle.h"
#include "pe1/room_m123_joint_triangle.h"

/* Particle drawn by the room_m123 pulsing sprite callback: it scales with a
 * cosine of its frame counter, then fades into a second blended sprite. */
typedef RoomM123BurstParticle RoomM123PulsingParticle;

extern u8 g_RoomM123GlowColorTrack[];

void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);
void LoadAverageShort12(void *, void *, int, int, void *);
void FieldFx_DrawGlowLine(void *, void *, void *, void *, int, int, int);

/* Joint triangle: three boss joint points joined by lines that pull toward a
 * fourth joint, then a spark that runs around the triangle edges. */
extern GteShortVector D_8018F1CC; /* joint offsets */
extern GteShortVector D_8018F1D4;
extern RenderColor D_8018F1DC;

#endif
