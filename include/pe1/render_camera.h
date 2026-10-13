#ifndef PE1_RENDER_CAMERA_H
#define PE1_RENDER_CAMERA_H

#include "pe1/gte_types.h"

extern GteMatrix g_CameraBasisMatrix;
extern s16 D_800BD002, D_800BD004, D_800BD006, D_800BD008, D_800BD00A;
extern s16 D_800BD00C, D_800BD00E, D_800BD010;
extern int D_800BD014, D_800BD018, D_800BD01C;
int Render_DrawSprite(void);

/* Camera initialization scratch: matrix words followed by projection distance. */
extern unsigned int D_800B89F8[];
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
