#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe1/game_state.h"
#include "pe1/scene_room.h"
Pe1GameState g_GameState;
SceneRoomSectorRange g_RoomSectorTable[SCENE_ROOM_COUNT];
int g_SceneDispatchCur;
u32 g_ScriptVariableBank[512];
static int loaded = 1, battle, *mode;
static PeHookFn on_random;
int pe_game_loaded(void) { return loaded; }
int pe_in_battle(void) { return battle; }
int Text_EncodeBase32(char *out, unsigned value) { (void)value; strcpy(out,"M0001"); return 0; }
int Text_ParseMapNumber(signed char *s) { (void)s; return 1; }
int pe_menu_register_choice(const char *p, const char *l, const char *d, int *v,
                            const char *const *n, unsigned c) {
    (void)p; (void)l; (void)d; (void)n; assert(c == 4); mode = v; return 0;
}
int pe_hook_add(int id, PeHookFn fn, void *u, int p, const char *m) {
    (void)u; (void)p; (void)m; assert(id == PE_HOOK_SCRIPT_RANDOM); on_random = fn; return 0;
}
void pe_battle_frequency_register(const char *);
static unsigned word(const unsigned char *p) { unsigned v; memcpy(&v,p,4); return v; }
static int seek64(FILE *f, long long offset) {
#ifdef _WIN32
    return _fseeki64(f, offset, SEEK_SET);
#else
    return fseeko(f, (off_t)offset, SEEK_SET);
#endif
}
static void user_data(FILE *f, unsigned lba, void *out, size_t bytes) {
    unsigned char *p = out;
    while (bytes) {
        size_t n = bytes > 2048 ? 2048 : bytes;
        assert(!seek64(f, (long long)lba * 2352 + 24));
        assert(fread(p,1,n,f) == n); p += n; bytes -= n; ++lba;
    }
}
static void real_rooms(const char *disc_path, const char *exe_path) {
    FILE *disc = fopen(disc_path,"rb"), *exe = fopen(exe_path,"rb");
    unsigned char pvd[2048], header[32], table[450*8];
    unsigned img = 0, checked = 0, matches = 0;
    assert(disc && exe);
    user_data(disc,16,pvd,sizeof pvd);
    unsigned root_lba = word(pvd+158), root_size = word(pvd+166);
    unsigned char *root = malloc(root_size); assert(root);
    user_data(disc,root_lba,root,root_size);
    for (size_t p=0; p+34 < root_size;) {
        if (!root[p]) { p = (p/2048+1)*2048; continue; }
        unsigned n = root[p]; assert(n <= root_size-p);
        if (root[p+32] >= 6 && !memcmp(root+p+33,"PE.IMG",6)) img = word(root+p+2);
        p += n;
    }
    free(root); assert(img);
    assert(fread(header,1,sizeof header,exe) == sizeof header);
    assert(!fseek(exe,(long)(0x80093378u-word(header+24)+0x800),SEEK_SET));
    assert(fread(table,1,sizeof table,exe) == sizeof table); fclose(exe);
    *mode = 1;
    for (unsigned i=0; i<450; ++i) {
        unsigned bits = word(table+i*8+4), sectors = bits >> 20;
        if (!sectors) continue;
        size_t bytes = (size_t)sectors * 2048;
        unsigned char *room = malloc(bytes); assert(room);
        user_data(disc,img+word(table+i*8)+(bits&255)+((bits>>8)&4095),room,bytes);
        g_GameState.loaded_scene_assets = (SceneAssetView *)room;
        g_RoomSectorTable[0].roomSectors = sectors;
        for (size_t p=0; p+20 <= bytes; p+=4) {
            if ((word(room+p)&0x1FFFu) != 26) continue;
            int start=0, end=(int)word(room+p+16), before=end;
            PeHookScriptRandom roll = {&start,&end,(const unsigned *)(room+p+20)};
            on_random(&roll,NULL);
            if (end != before) { assert(end == before/2); ++matches; }
        }
        ++checked; free(room);
    }
    fclose(disc); assert(matches);
    printf("Original disc data: %u room blobs checked; %u encounter-roll sequences matched.\n",checked,matches);
}
int main(int argc, char **argv) {
    unsigned code[512] = {0};
    pe_battle_frequency_register("pe_qol");
    g_GameState.loaded_scene_assets=(SceneAssetView *)code;
    g_RoomSectorTable[0].roomSectors=1;
    code[0]=26|(3<<13)|(3<<17); code[2]=2; code[4]=256;
    code[5]=9|(4<<13)|(3<<23); code[7]=10; code[9]=2;
    code[11]=5|(2<<13); code[15]=0x55|(1<<13);
    int start=0,end=256; PeHookScriptRandom roll={&start,&end,code+5};
    assert(*mode==0); on_random(&roll,NULL); assert(end==256);
    *mode=1; on_random(&roll,NULL); assert(end==128 && code[4]==256);
    *mode=2; end=256; on_random(&roll,NULL); assert(end==64);
    *mode=3; end=256; on_random(&roll,NULL); assert(end==1);
    *mode=0; end=256; on_random(&roll,NULL); assert(end==256);
    *mode=1; code[15]=0; on_random(&roll,NULL); assert(end==256); code[15]=0x55|(1<<13);
    code[7]=9; on_random(&roll,NULL); assert(end==256); code[7]=10;
    battle=1; on_random(&roll,NULL); assert(end==256); battle=0;
    loaded=0; on_random(&roll,NULL); assert(end==256); loaded=1;
    g_ScriptVariableBank[10]=0x80000000u; on_random(&roll,NULL); assert(end==256); g_ScriptVariableBank[10]=0;
    roll.next=code+1; on_random(&roll,NULL); assert(end==256);
    roll.next=code+511; on_random(&roll,NULL); assert(end==256);
    roll.next=code+5; end=2; *mode=2; on_random(&roll,NULL); assert(end==1);
    puts("Passed frequency modes, Normal restoration, unrelated RNG rejection, truncation bounds, and battle/skip guards.");
    if (argc==3) real_rooms(argv[1],argv[2]);
    return 0;
}
