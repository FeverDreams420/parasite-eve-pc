#ifndef PE1_RENDER_CAMERA_H
#define PE1_RENDER_CAMERA_H

#include "pe1/gte_types.h"

extern GteMatrix g_CameraBasisMatrix;
int Render_DrawSprite(void);

/* Camera initialization scratch: matrix words followed by projection distance. */
extern u32 g_EntityRenderScratch[];
int Render_InitViewState(void *matrix, void *screen);
int Render_DrawMeshList(void *buffer, void **end);
int Render_ResetAllEntries(void);
void SetGeomScreen(int distance);
int Render_PrepareFrame(void);

int CdRom_SetSeekPos(unsigned int arg0);
int Render_LoadGeomState(int index);
int Render_SetFadeColour(unsigned int arg0);
int CdRom_SetScreenPos(int arg0, int arg1, int arg2, int arg3, unsigned short arg4);

#endif
