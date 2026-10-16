#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <SDL3/SDL.h>
#include <psyz/audio.h>
#include "pe1/boot.h"
#include "pe1/boot_disc_check.h"
#include "pe1/boot_display.h"
#include "pe1/boot_services.h"
#include "pe1/field_bg_load.h"
#include "pe1/game_state.h"
#include "pe1/game_timers.h"
#include "pe1/psyq_gpu.h"
#include "pe1/scene_dispatch.h"

#define STATE_FLAG_IMAGE_BUSY 0x00100000U
#define STATE_FLAG_READY 0x00000100U

int main(int argc,char **argv) {
    int mode;
    u32 token;
    Pe1GameState *state;
    PeDisc images[2]={{0}};
    const char *paths[2]={NULL,NULL};
    int result=EXIT_FAILURE;
    for(int arg=1;arg<argc;++arg){
        if(!strcmp(argv[arg],"--help")){
            printf("Usage: %s --disc <USA Disc 1 CUE/BIN> [--disc2 <USA Disc 2 CUE/BIN>] [--headless] [--frame-limit N] [--dump-vram PATH] [--pad-at VBLANK:BUTTON_MASK ...]\n",argv[0]);
            printf("Headless fixtures only: --diagnostic-hp-at VBLANK:EXPECTED_HP:VALUE (requires --frame-limit; output is marked coverage-only)\n");
            return EXIT_SUCCESS;
        }
        if(!strcmp(argv[arg],"--headless")){
            if(!SDL_SetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS","1",true))return EXIT_FAILURE;
        }else if(!strcmp(argv[arg],"--frame-limit")){
            char *end;errno=0;
            unsigned long limit=arg+1<argc?strtoul(argv[++arg],&end,10):0;
            if(!limit || limit>INT_MAX || errno || *end){fprintf(stderr,"Invalid frame limit\n");return EXIT_FAILURE;}
            g_BootHostVBlankLimit=(u32)limit;
        }else if(!strcmp(argv[arg],"--dump-vram")){
            if(arg+1>=argc || g_BootHostVramPath){fprintf(stderr,"Missing or duplicate capture path\n");return EXIT_FAILURE;}
            g_BootHostVramPath=argv[++arg];
        }else if(!strcmp(argv[arg],"--pad-at")){
            if(arg+1>=argc){fprintf(stderr,"Invalid input schedule\n");return EXIT_FAILURE;}
            char *end,*mask_end;errno=0;unsigned long tick=strtoul(argv[++arg],&end,10);
            if(!tick || tick>INT_MAX || errno || *end!=':'){fprintf(stderr,"Invalid input frame\n");return EXIT_FAILURE;}
            unsigned long buttons=strtoul(end+1,&mask_end,0);
            if(mask_end==end+1 || *mask_end || errno || buttons>65535 ||
               (g_BootHostPadEventCount && tick<g_BootHostPadEvents[g_BootHostPadEventCount-1].vblank)){
                fprintf(stderr,"Invalid input buttons/order\n");return EXIT_FAILURE;
            }
            if(!g_BootHostPadEvents){
                /* There cannot be more events than command-line arguments. */
                g_BootHostPadEvents=calloc((size_t)argc,sizeof(*g_BootHostPadEvents));
                if(!g_BootHostPadEvents){fprintf(stderr,"Cannot allocate input schedule\n");return EXIT_FAILURE;}
            }
            g_BootHostPadEvents[g_BootHostPadEventCount++]=(BootHostPadEvent){(u32)tick,(u16)buttons};
        }else if(!strcmp(argv[arg],"--diagnostic-hp-at")){
            if(arg+1>=argc){fprintf(stderr,"Invalid diagnostic HP schedule\n");return EXIT_FAILURE;}
            const char *cursor=argv[++arg];unsigned long values[3];
            for(unsigned field=0;field<3;++field){
                char *end;errno=0;
                if(*cursor<'0' || *cursor>'9'){fprintf(stderr,"Invalid diagnostic HP event\n");return EXIT_FAILURE;}
                values[field]=strtoul(cursor,&end,10);
                if(errno || (field?values[field]>USHRT_MAX:(!values[field] || values[field]>INT_MAX)) ||
                   *end!=(field==2?'\0':':')){fprintf(stderr,"Invalid diagnostic HP event\n");return EXIT_FAILURE;}
                cursor=end+1;
            }
            if(g_BootHostHealthEventCount && values[0]<=g_BootHostHealthEvents[g_BootHostHealthEventCount-1].vblank){
                fprintf(stderr,"Diagnostic HP events must have strictly increasing frames\n");return EXIT_FAILURE;
            }
            if(!g_BootHostHealthEvents){
                g_BootHostHealthEvents=calloc((size_t)argc,sizeof(*g_BootHostHealthEvents));
                if(!g_BootHostHealthEvents){fprintf(stderr,"Cannot allocate diagnostic HP schedule\n");return EXIT_FAILURE;}
            }
            g_BootHostHealthEvents[g_BootHostHealthEventCount++]=(BootHostHealthEvent){(u32)values[0],(u16)values[1],(u16)values[2]};
        }else if(!strcmp(argv[arg],"--disc") || !strcmp(argv[arg],"--disc2")){
            int slot=!strcmp(argv[arg],"--disc2");
            if(paths[slot] || arg+1>=argc){fprintf(stderr,"Missing or duplicate image argument\n");return EXIT_FAILURE;}
            paths[slot]=argv[++arg];
        }else{fprintf(stderr,"Unknown argument: %s\n",argv[arg]);return EXIT_FAILURE;}
    }
    if(!paths[0]){fprintf(stderr,"Supply --disc with a USA CUE/BIN image\n");return EXIT_FAILURE;}
    if(g_BootHostVramPath && !g_BootHostVBlankLimit){fprintf(stderr,"VRAM capture requires --frame-limit\n");return EXIT_FAILURE;}
    if(g_BootHostHealthEventCount){
        const char *headless=SDL_GetEnvironmentVariable(SDL_GetEnvironment(),"PSYZ_HEADLESS");
        if(!headless || strcmp(headless,"1") || !g_BootHostVBlankLimit ||
           g_BootHostHealthEvents[g_BootHostHealthEventCount-1].vblank>g_BootHostVBlankLimit){
            fprintf(stderr,"Diagnostic HP schedule requires headless mode and a frame limit covering every event\n");return EXIT_FAILURE;
        }
    }
    for(unsigned slot=0;slot<2;++slot){
        if(!paths[slot])continue;
        if(!pe_disc_open(&images[slot],paths[slot])){
            fprintf(stderr,"Cannot open image: %s\n",images[slot].error);goto shutdown;
        }
        CdRom_BindImage(&images[slot],NULL);
        int mounted=Asset_OpenPeImage();
        unsigned identity=g_PeImageMountFlags;
        if(mounted || (identity!=1 && identity!=2) || g_CdImageVolumes[identity-1]){
            fprintf(stderr,"Invalid or duplicate USA disc image (status %d)\n",mounted);goto shutdown;
        }
        g_CdImageVolumes[identity-1]=&images[slot];
    }
    if(!g_CdImageVolumes[0]){fprintf(stderr,"USA Disc 1 is required for startup\n");goto shutdown;}
    CdRom_BindImage(g_CdImageVolumes[0],NULL);

    g_BootHostActive=1;
    Boot_InitSystem();
    mode = 0;

    for (;;) {
        state = &g_GameState;
        Boot_SyncShutdown();

        int image_status = Asset_OpenPeImage();
        if (image_status != 0) {
            fprintf(stderr, "Game image mount failed: %d\n", image_status);
            goto shutdown;
        }

        Boot_InitGameState();
        Boot_InitSubsystems();
        int startup_status = Scene_LoadSceneData();
        if (startup_status < 0) {
            fprintf(stderr, "Scene startup failed: %d\n", startup_status);
            goto shutdown;
        }

        g_SceneDispatchToken = 0xA9400048;

        do {
            if (g_GameState.flags & STATE_FLAG_IMAGE_BUSY) {
                int display_status = Render_InitDisplayLists(mode);
                if (display_status < 0) {
                    fprintf(stderr, "Game image selection failed: %d\n", display_status);
                    goto shutdown;
                }
                g_GameState.flags &= ~STATE_FLAG_IMAGE_BUSY;
            }

            Scene_LoadFieldBg();
            token = g_SceneDispatchToken;
            g_SceneDispatchCur = token;

            switch (token) {
            case 0xA8000048: {
                int overlay_status = Boot_LoadOverlayTables();
                if (overlay_status < 0) {
                    fprintf(stderr, "Boot assets failed: %d\n", overlay_status);
                    goto shutdown;
                }
                if(FxCommon_RunFrameLoop()<0){
                    fprintf(stderr,"Effect scene startup failed\n");goto shutdown;
                }
                g_GameState.flags |= 1;
                break;
            }
            case 0xAA108448:
                if ((g_GameState.display_list_modes & 2) == 0) {
                    mode = 2;
                    g_GameState.flags |= STATE_FLAG_IMAGE_BUSY;
                    break;
                }
                if (CdRom_LoadBootAudio() < 0) {
                    fprintf(stderr, "Boot display assets failed\n");
                    goto shutdown;
                }
                if(BootDisplay_RunDisplay()<0){
                    fprintf(stderr,"Boot display failed\n");goto shutdown;
                }
                g_SceneDispatchToken = 0xA80830C8;
                g_GameState.flags |= 1;
                break;
            case 0xA9400048: {
                int title_status = CdRom_LoadInitialImage();
                if (title_status < 0) {
                    fprintf(stderr, "Title assets failed: %d\n", title_status);
                    goto shutdown;
                }
                Draw_InitDisplay(MenuCard_RunListScreen());
                g_GameState.flags |= 3;
                break;
            }
            default:
                {int frame_status=Boot_RunFrame();
                if(frame_status<0){fprintf(stderr,"Scene frame startup failed: %d\n",frame_status);goto shutdown;}}
                break;
            }

            if (g_SceneDispatchToken != 0xA80651C8 && g_SceneDispatchToken != 0xA8065248 &&
                g_SceneDispatchToken != 0xA80652C8 && g_SceneDispatchToken != 0xA80660C8 &&
                g_SceneDispatchToken != 0xA8066148 && g_SceneDispatchToken != 0xA80661C8 &&
                g_SceneDispatchToken != 0xA8066348) {
                if (g_PlayTimeFrameCounter < 600) {
                    if ((g_GameState.display_list_modes & 1) == 0) {
                        mode = 1;
                        g_GameState.flags |= STATE_FLAG_IMAGE_BUSY;
                    }
                } else if ((g_GameState.display_list_modes & 2) == 0) {
                    mode = 2;
                    g_GameState.flags |= STATE_FLAG_IMAGE_BUSY;
                }
            }
        } while ((state->flags & STATE_FLAG_READY) == 0);

        VSync(0);
        SetDispMask(0);
        state->flags &= ~STATE_FLAG_READY;
    }
shutdown:
    if(g_BootDisplayPlaybackActive)BootDisplay_WaitForCdAndRestoreCallbacks();
    Psyz_AudioDestroy();
    CdRom_BindImage(NULL,NULL);
    g_CdImageVolumes[0]=g_CdImageVolumes[1]=NULL;
    pe_disc_close(&images[0]);pe_disc_close(&images[1]);
    free(g_BootHostHealthEvents);g_BootHostHealthEvents=NULL;g_BootHostHealthEventCount=g_BootHostHealthApplied=0;
    free(g_BootHostPadEvents);g_BootHostPadEvents=NULL;g_BootHostPadEventCount=0;
    return result;
}
