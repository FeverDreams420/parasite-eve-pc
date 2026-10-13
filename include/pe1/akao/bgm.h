#ifndef PE1_AKAO_BGM_H
#define PE1_AKAO_BGM_H

/* First declaration of this symbol for cdrom/track_read.c. Two pointers are
 * 8 bytes, so cc1 -G8 reloads the word on every read. akao.h's unsized array
 * is large: the four reads in CdRom_StepReadState would share one address
 * register, and the object would no longer match. Include this before akao.h. */
extern void *g_AkaoBgmHandle[2];

#endif
