#ifndef PE_HOOKS_H
#define PE_HOOKS_H

/* Mod hook points. Each PE_HOOK line in the game hands one moment to the mod
 * host (mods/host), with a small context struct of pointers to the values a
 * mod may read or change. Without PE_MODS the macro is 0 and the game is the
 * unchanged original. A non-zero result means a mod handled the moment and
 * the caller skips its own code; only hooks documented as replaceable check it. */

typedef enum PeHookId {
    PE_HOOK_INIT,        /* PeHookInit: once, before the first scene */
    PE_HOOK_SCENE_START, /* NULL: a field scene finished loading, before its first frame */
    PE_HOOK_FRAME,       /* NULL: every field frame, after pad input is read */
    PE_HOOK_INV_COMMAND, /* PeHookInvCommand: Inv_DispatchCommand entry; replaceable */
    PE_HOOK_SAVE_READ,   /* u8 *: the 8 KB card block of a save being loaded */
    PE_HOOK_PAD_READ,    /* NULL: Field_HandleStateTransition, before it reads g_PadPackets */
    PE_HOOK_SCRIPT_RANDOM, /* PeHookScriptRandom: local RNG bounds, before a script roll */
    PE_HOOK_COUNT
} PeHookId;

/* next points just past the current instruction in native loaded bytecode. */
typedef struct PeHookScriptRandom {
    int *start, *end;
    const unsigned int *next;
} PeHookScriptRandom;

typedef struct PeHookInvCommand {
    int command, value;
    int result; /* a handler returning nonzero replaces the command with this result */
} PeHookInvCommand;

typedef struct PeHookInit {
    int interactive;        /* 0 for headless runs: no window, no menus */
    const char *prefs_file; /* per-user preferences file; mods keep their files beside it */
} PeHookInit;

#ifdef PE_MODS
int pe_host_init(const PeHookInit *init);
int pe_hook_dispatch(int id, void *ctx);
#define PE_HOOK(id, ctx) \
    ((id) == PE_HOOK_INIT ? pe_host_init((const PeHookInit *)(const void *)(ctx)) : pe_hook_dispatch((id), (ctx)))
#else
#define PE_HOOK(id, ctx) ((void)0, 0)
#endif

#endif
