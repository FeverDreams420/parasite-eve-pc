#include "automation.h"
#include "boot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *disc = getenv("PE_TEST_DISC");
    if (!disc || !*disc) {
        fprintf(stderr, "PE_TEST_DISC is required for the boot replay\n");
        return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    }
    if (argc != 3 && !(argc == 4 && !strcmp(argv[3], "--menu"))) return 1;
    pe_runtime_set_headless(1);
    pe_runtime_set_replay_timeout(5000);
    if (!pe_runtime_load_replay(argv[1]) || !pe_runtime_open_trace(argv[2])) {
        fprintf(stderr, "%s\n", pe_runtime_error());
        return pe_runtime_finish(1);
    }
    return pe_runtime_finish(pe_boot_game(disc, argc == 4));
}
