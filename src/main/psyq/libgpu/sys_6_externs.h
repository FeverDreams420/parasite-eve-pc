#ifndef PSYQ_LIBGPU_GPU_BUILD_EXTERNS_H
#define PSYQ_LIBGPU_GPU_BUILD_EXTERNS_H

extern unsigned int *g_GpuGp1Ptr;
extern volatile unsigned int *g_GpuDmaControlRegPtr;
extern volatile unsigned int *g_GpuOtDmaChcrPtr;
extern volatile unsigned int *g_GpuOtDmaMadrPtr;
extern volatile unsigned int *g_GpuOtDmaBcrPtr;
extern short g_GpuMaxWidth;
extern short g_GpuMaxHeight;
extern unsigned int g_GpuVramFillPacket[];
extern unsigned int g_GpuVramFillTailPacket[];
extern unsigned int *g_GpuGp0Ptr;
extern unsigned char g_GpuControlRegMirror[];
extern unsigned int *g_GpuDmaBcrPtr;

#endif /* PSYQ_LIBGPU_GPU_BUILD_EXTERNS_H */
