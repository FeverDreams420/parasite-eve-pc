#ifdef PE_HOST_SETTINGS_TEST
#undef NDEBUG
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
static bool TestPresent(SDL_Renderer *renderer);
#define SDL_RenderPresent TestPresent
#define main original_game_main
#include "../src/main/boot/main.c"
#undef main
#undef SDL_RenderPresent
static int presents;
static const char *test_directory;
static bool TestPresent(SDL_Renderer *renderer) {
    ++presents;
    assert(Psyz_AudioIsPaused());
    assert(Psyz_AudioWaitForTimer()==0);
    if(presents==1 || presents==4 || presents==7){
        SDL_Surface *surface=SDL_RenderReadPixels(renderer,NULL);assert(surface);
        char path[4096];snprintf(path,sizeof(path),"%s/settings-%d.bmp",test_directory,presents);
        assert(SDL_SaveBMP(surface,path));SDL_DestroySurface(surface);
    }
    return SDL_RenderPresent(renderer);
}
static void Key(SDL_Scancode key){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=key;assert(SDL_PushEvent(&e));}
static unsigned requested_image,request_count;
static const char *request_path;
static int SupplyImage(unsigned identity){
    requested_image=identity;++request_count;
    return request_path?Host_InstallImage(identity,request_path):0;
}
static void DiscImages(void){
    const char *paths[2]={getenv("PE_TEST_DISC"),getenv("PE_TEST_DISC2")};
    if(!paths[0] || !paths[1]){
        assert(!getenv("PE_REQUIRE_GAME_ASSETS"));return;
    }
    PeDisc images[2]={{0}};host_images=images;
    assert(pe_disc_open(&images[0],paths[0]));
    g_CdImageVolumes[0]=&images[0];g_CdImageVolumes[1]=NULL;
    CdRom_BindImage(&images[0],NULL);assert(!Asset_OpenPeImage());
    unsigned identity=0;Pe1GameState before=g_GameState;
    unsigned flags=g_PeImageMountFlags;u32 base=g_PeImageBaseLba;
    assert(!Asset_ValidatePeImage(&images[0],&identity) && identity==1);
    assert(!Host_InstallImage(2,paths[0]));
    assert(!g_CdImageVolumes[1] && !images[1].stream);
    assert(g_CdImage==&images[0] && g_PeImageMountFlags==flags && g_PeImageBaseLba==base);
    assert(!memcmp(&before,&g_GameState,sizeof(before)));
    assert(Host_InstallImage(2,paths[1]));
    assert(g_CdImageVolumes[1]==&images[1] && !strcmp(host_options.discs[1],paths[1]));
    assert(g_CdImage==&images[0] && g_PeImageMountFlags==flags && g_PeImageBaseLba==base);
    assert(!memcmp(&before,&g_GameState,sizeof(before)));
    pe_disc_close(&images[1]);g_CdImageVolumes[1]=NULL;
    g_CdImageRequest=SupplyImage;request_path=NULL;request_count=0;
    assert(Render_InitDisplayLists(2)==-4 && requested_image==2 && request_count==1);
    assert(g_CdImage==&images[0] && images[0].stream);
    request_path=paths[0];assert(Render_InitDisplayLists(2)==-4 && !g_CdImageVolumes[1]);
    assert(SDL_SetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS","1",true));
    Psyz_VideoSetVsyncMode(PSYZ_VSYNC_LIMITLESS);ResetGraph(0);
    request_path=paths[1];assert(!Render_InitDisplayLists(2));
    assert(requested_image==2 && g_CdImage==g_CdImageVolumes[1] && images[0].stream);
    unsigned requests=request_count;assert(!Render_InitDisplayLists(1) && request_count==requests);
    CdRom_BindImage(NULL,NULL);g_CdImageVolumes[0]=g_CdImageVolumes[1]=NULL;
    pe_disc_close(&images[0]);pe_disc_close(&images[1]);
    /* Starting with Disc 2 is valid; the original check requests Disc 1. */
    assert(pe_disc_open(&images[0],paths[1]));g_CdImageVolumes[1]=&images[0];
    CdRom_BindImage(&images[0],NULL);assert(!Asset_OpenPeImage() && g_PeImageMountFlags==2);
    request_path=paths[0];assert(!Render_InitDisplayLists(1));
    assert(requested_image==1 && g_CdImage==g_CdImageVolumes[0] && images[0].stream);
    assert(!strcmp(host_options.discs[0],paths[0]) && !strcmp(host_options.discs[1],paths[1]));
    CdRom_BindImage(NULL,NULL);g_CdImageVolumes[0]=g_CdImageVolumes[1]=NULL;
    g_CdImageRequest=NULL;host_images=NULL;pe_disc_close(&images[0]);pe_disc_close(&images[1]);
}
static int SDLCALL ClosePanel(void *unused){
    (void)unused;SDL_Delay(100);Key(SDL_SCANCODE_ESCAPE);return 0;
}
static int SDLCALL Drive(void *unused){
    (void)unused;SDL_Delay(150);
    Key(SDL_SCANCODE_RIGHT); /* graphics scale 1 -> 2 */
    Key(SDL_SCANCODE_TAB); /* controls */
    Key(SDL_SCANCODE_DOWN); /* L2 */
    Key(SDL_SCANCODE_RETURN);Key(SDL_SCANCODE_F); /* rebind */
    Key(SDL_SCANCODE_TAB); /* audio */
    Key(SDL_SCANCODE_LEFT); /* 100 -> 95 */
    Key(SDL_SCANCODE_ESCAPE);return 0;
}
int main(int argc,char **argv){
    assert(argc==2);test_directory=argv[1];assert(SDL_CreateDirectory(test_directory));
    assert(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy"));
    assert(SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software"));
    assert(SDL_Init(SDL_INIT_VIDEO));
    Host_InitOptions(0);
    Psyz_VideoSetVsyncMode(PSYZ_VSYNC_OFF);
    unsigned frozen=(unsigned)Psyz_VideoVSync(-1);
    Psyz_PauseHostClock_SDL3(true);SDL_Delay(80);
    assert((unsigned)Psyz_VideoVSync(-1)==frozen);
    Psyz_PauseHostClock_SDL3(true);SDL_Delay(40);Psyz_PauseHostClock_SDL3(false);
    assert((unsigned)Psyz_VideoVSync(-1)==frozen);
    Psyz_PauseHostClock_SDL3(false);
    assert((unsigned)Psyz_VideoVSync(-1)-frozen<=1);
    assert(Psyz_SetKeyboardBinding_SDL3(16,SDL_SCANCODE_A)==-1);
    assert(Psyz_SetKeyboardBinding_SDL3(0,SDL_SCANCODE_ESCAPE)==-1);
    assert(Psyz_SetGamepadBinding_SDL3(16,0)==-1);
    assert(Psyz_SetGamepadBinding_SDL3(0,-1)==-1);
    assert(Psyz_AudioSetOutputVolume(101)==-1);
    host_options.enabled=1;
    snprintf(host_options.user_file,sizeof(host_options.user_file),"%s/preferences.toml",test_directory);
    snprintf(host_options.discs[0],sizeof(host_options.discs[0]),"/tmp/zażółć # \"quotes\" \\ disc.cue");
    assert(Host_SaveOptions());
    host_options.discs[0][0]=0;Host_LoadOptions(host_options.user_file);
    assert(!strcmp(host_options.discs[0],"/tmp/zażółć # \"quotes\" \\ disc.cue"));
    char decoded[80];assert(Host_String("'C:\\game\\disc.cue' # comment",decoded,sizeof(decoded)));
    assert(!strcmp(decoded,"C:\\game\\disc.cue"));
    char invalid[4096];snprintf(invalid,sizeof(invalid),"%s/invalid.toml",test_directory);
    FILE *f=fopen(invalid,"w");assert(f);
    fputs("[graphics]\ninternal_resolution = 9\nfullscreen = false\n[audio]\nvolume = 101\n[keyboard]\nL2 = 41\n[discs]\ndisc1 = \"unterminated\n",f);fclose(f);
    Host_LoadOptions(invalid);
    assert(host_options.scale==1 && host_options.volume==100);
    assert(!strcmp(host_options.discs[0],"/tmp/zażółć # \"quotes\" \\ disc.cue"));
    assert(SDL_SetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_AUDIO_CLOCK","video",true));
    assert(Psyz_AudioInit()==0 && !Psyz_AudioIsPaused());
    int original_key=host_options.keys[0];
    SDL_Thread *thread=SDL_CreateThread(Drive,"settings input",NULL);assert(thread);
    SDL_Event open={0};open.type=SDL_EVENT_KEY_DOWN;open.key.scancode=SDL_SCANCODE_ESCAPE;
    assert(Host_SettingsEvent(&open));SDL_WaitThread(thread,NULL);
    assert(!Psyz_AudioIsPaused());
    assert(host_options.scale==2 && host_options.volume==95);
    assert(host_options.keys[0]==SDL_SCANCODE_F && original_key!=SDL_SCANCODE_F);
    assert(Psyz_GetKeyboardBinding_SDL3(0)==SDL_SCANCODE_F);
    assert(Psyz_AudioGetOutputVolume()==95);
    host_options.scale=1;host_options.volume=100;host_options.keys[0]=original_key;
    Host_LoadOptions(host_options.user_file);
    assert(host_options.scale==2 && host_options.volume==95 && host_options.keys[0]==SDL_SCANCODE_F);
    SDL_Event ignored={0};ignored.type=SDL_EVENT_KEY_UP;assert(!Host_SettingsEvent(&ignored));
    assert(Psyz_SetOverlayEventFilter_SDL3(Host_SettingsEvent)==NULL);
    assert(Psyz_SetOverlayEventFilter_SDL3(NULL)==Host_SettingsEvent);
    /* Multi-selection callback owns paths and rejects a third image. */
    HostDiscDialog dialog={0};const char *files[]={"second.cue","first.cue",NULL};
    Host_DiscChosen(&dialog,files,0);assert(SDL_GetAtomicInt(&dialog.done));
    assert(!dialog.failed && !strcmp(dialog.paths[1],"first.cue"));
    SDL_free(dialog.paths[0]);SDL_free(dialog.paths[1]);
    memset(&dialog,0,sizeof(dialog));const char *three[]={"1.cue","2.cue","3.cue",NULL};
    Host_DiscChosen(&dialog,three,0);assert(dialog.failed);
    SDL_free(dialog.paths[0]);SDL_free(dialog.paths[1]);
    memset(&dialog,0,sizeof(dialog));const char *cancel[]={NULL};
    Host_DiscChosen(&dialog,cancel,0);assert(!dialog.failed && !dialog.paths[0]);
    Host_DiscChosen(&dialog,NULL,0);assert(dialog.failed);
    Psyz_AudioPause();
    thread=SDL_CreateThread(ClosePanel,"close paused settings",NULL);assert(thread);
    assert(Host_SettingsEvent(&open));SDL_WaitThread(thread,NULL);
    assert(Psyz_AudioIsPaused());Psyz_AudioDestroy();
    DiscImages();
    SDL_Quit();puts("PASS: both image identities, missing-image checks, additive mounting, TOML roundtrip, invalid values, modal graphics/controls/audio, persistence, ESC consumption, multi-disc callback");return 0;
}

#else
#include <psyz/overlay_sdl3.h>
#include "pe1/pad_state.h"
#include "pe1/boot.h"
#include <psyz/input.h>
#include "pe1/scene_flags.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include "pe1/battle.h"
static unsigned calls;
static void trigger(struct BattleEntity *ctx, int *index) { ++calls; ctx->motionX+=1; *index=1; }
static void fallback(struct BattleEntity *ctx, int *index) { (void)index; ++calls; ctx->motionX+=10; }
int main(void)
{
    if(!SDL_Init(SDL_INIT_EVENTS))return 10;
    Boot_ResetPadInput();Boot_ResetPadInput();
    const SDL_Scancode keys[]={SDL_SCANCODE_RETURN,SDL_SCANCODE_KP_ENTER};
    for(unsigned i=0;i<2;++i){
        SDL_Event event={.type=SDL_EVENT_KEY_DOWN};event.key.scancode=keys[i];
        if(!SDL_PushEvent(&event) || (g_PadPackets[0].buttons&8))return 11;
        if((g_PadPackets[0].header&0xf0ff)!=0x4000)return 12;
        event.type=SDL_EVENT_KEY_UP;
        if(!SDL_PushEvent(&event) || !(g_PadPackets[0].buttons&8))return 13;

    }
    if(Psyz_SetKeyboardBinding_SDL3(11,SDL_SCANCODE_F))return 40;
    Boot_ResetPadInput(); /* Reset must not undo an explicit rebind. */
    if(Psyz_GetKeyboardBinding_SDL3(11)!=SDL_SCANCODE_F)return 45;
    SDL_Event remapped={.type=SDL_EVENT_KEY_DOWN};remapped.key.scancode=SDL_SCANCODE_RETURN;
    SDL_PushEvent(&remapped);if(!(g_PadPackets[0].buttons&8))return 41;
    remapped.type=SDL_EVENT_KEY_UP;SDL_PushEvent(&remapped);
    remapped.type=SDL_EVENT_KEY_DOWN;remapped.key.scancode=SDL_SCANCODE_F;
    SDL_PushEvent(&remapped);if(g_PadPackets[0].buttons&8)return 42;
    remapped.type=SDL_EVENT_KEY_UP;SDL_PushEvent(&remapped);
    if(Psyz_SetKeyboardBinding_SDL3(11,SDL_SCANCODE_RETURN))return 43;
    SDL_Event focus={.type=SDL_EVENT_WINDOW_FOCUS_LOST};SDL_PushEvent(&focus);
    SDL_Event key={.type=SDL_EVENT_KEY_DOWN};key.key.scancode=SDL_SCANCODE_RETURN;SDL_PushEvent(&key);
    if(!(g_PadPackets[0].buttons&8))return 15;
    focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;SDL_PushEvent(&focus);SDL_PushEvent(&key);
    if(g_PadPackets[0].buttons&8)return 16;
    key.type=SDL_EVENT_KEY_UP;SDL_PushEvent(&key);
    /* Exercise native controller polling without opening a video device. */
    const char *headless=SDL_GetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS");
    char *saved_headless=headless?SDL_strdup(headless):NULL;
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS","",true);
    g_BootHostActive=1;Boot_ResetPadInput();
    char controller[PSYZ_PAD_BUF_LEN]={0};controller[1]=0x41;controller[2]=(char)0xff;controller[3]=(char)0xbf;
    Psyz_PadsSet(0,controller,sizeof(controller));Field_HandleStateTransition();
    if(g_PadPackets[0].buttons&0x4000)return 44;
    g_BootHostActive=0;Boot_ResetPadInput();
    if(saved_headless){SDL_SetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS",saved_headless,true);SDL_free(saved_headless);}
    else SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS");
    puts("SDL keyboard watch: Return/keypad Enter reach original START; release, focus and repeated reset passed");
    SceneFlagRecord rules[]={{1|4,3,4,trigger},{0,0,0,fallback}};
    SceneFlagRecord *table[]={rules,0};
    int mode=0;BattleEntity actor={0};
#define value actor.motionX
    g_FieldPadBits=3; g_FieldPadPressed=0;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=10 || calls!=1) return 1;
    mode=0; value=0; calls=0; g_FieldPadPressed=4;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=1 || calls!=1 || mode!=1) return 2;
    /* Callback mode changes do not redirect the current record walk. */
    SceneFlagRecord continued[]={{0x20|0x10,0,0,trigger},{0x20,0,0,trigger},{0,0,0,fallback}};
    table[0]=continued; mode=0;value=0;calls=0;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=2 || calls!=2) return 3;
    rules[0].flags=2|8; rules[0].mask_any=4; table[0]=rules;
    mode=0;value=0;calls=0;g_FieldPadBits=4;g_FieldPadReleased=4;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=1 || calls!=1) return 4;
    puts("Original input rule masks, callback continuation and fallback passed");
    return 0;
}

#endif
