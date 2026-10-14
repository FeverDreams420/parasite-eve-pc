#ifndef PE_PC_BOOT_H
#define PE_PC_BOOT_H
#include "disc.h"
int pe_boot_intro(const char *disc_path);
int pe_boot_game(const char *disc_path, int skip_intro);
/* Borrow a mounted source; caller retains ownership until boot returns. */
int pe_boot_game_source(PeDisc *disc, int skip_intro);
#endif
