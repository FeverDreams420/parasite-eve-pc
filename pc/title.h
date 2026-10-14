#ifndef PE_PC_TITLE_H
#define PE_PC_TITLE_H
#include "disc.h"
#include <SDL3/SDL.h>
/* Zero means quit; 1 means New Game, 2 means Tutorial. Gameplay is not yet linked. */
int pe_title_menu(PeDisc *disc, SDL_Renderer *renderer, char *error, size_t error_size);
#endif
