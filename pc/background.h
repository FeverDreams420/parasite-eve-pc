#ifndef PE_PC_BACKGROUND_H
#define PE_PC_BACKGROUND_H
#include "camera.h"
typedef struct PeBackground { PeCameraBank bank;uint32_t entries;unsigned count; } PeBackground;
typedef struct PeBackgroundEntry { uint32_t flags,positions,textures;uint16_t screen[2],modulus[2],tiles;uint8_t group; } PeBackgroundEntry;
typedef struct PeBackgroundTile { uint32_t position,texture[2];uint16_t x,y,clut,page;uint8_t u,v,semitransparent; } PeBackgroundTile;
typedef struct PeBackgroundDrawTile { PeBackgroundTile tile;int16_t x,y;uint16_t depth;unsigned entry,index; } PeBackgroundDrawTile;
typedef struct PeBackgroundMotion { uint8_t flags;uint16_t modulus[2],fraction[2];int16_t base[2],position[2],speed[2]; } PeBackgroundMotion;
int pe_background_open(const PeCameraBank *bank,PeBackground *background);
int pe_background_entry(const PeBackground *background,unsigned index,PeBackgroundEntry *entry);
int pe_background_tile(const PeBackground *background,unsigned entry,unsigned tile,PeBackgroundTile *output);
int pe_background_draw_tiles(const PeBackground *background,unsigned group,const int16_t display[2],
                              PeBackgroundDrawTile *output,unsigned capacity,unsigned *written);
int pe_background_motion(const PeBackground *background,unsigned index,PeBackgroundMotion *motion);
int pe_background_motion_step(PeBackgroundMotion *motion,const PeCameraScroll *camera,uint32_t scene_flags);
int pe_background_draw_tiles_moving(const PeBackground *background,const PeBackgroundMotion *motion,unsigned motion_count,
                                     unsigned group,const int16_t display[2],PeBackgroundDrawTile *output,unsigned capacity,unsigned *written);
#endif
