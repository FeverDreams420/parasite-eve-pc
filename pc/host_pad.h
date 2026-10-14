#ifndef PE_HOST_PAD_H
#define PE_HOST_PAD_H
#include <SDL3/SDL.h>
/* Host I/O only: original game code decodes this digital PS1 packet. */
void pe_host_pad_reset(void);
void pe_host_pad_event(const SDL_Event *event);
#endif
