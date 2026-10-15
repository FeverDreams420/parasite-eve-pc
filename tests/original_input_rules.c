#include "pe1/pad_state.h"
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
    SDL_Event focus={.type=SDL_EVENT_WINDOW_FOCUS_LOST};SDL_PushEvent(&focus);
    SDL_Event key={.type=SDL_EVENT_KEY_DOWN};key.key.scancode=SDL_SCANCODE_RETURN;SDL_PushEvent(&key);
    if(!(g_PadPackets[0].buttons&8))return 15;
    focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;SDL_PushEvent(&focus);SDL_PushEvent(&key);
    if(g_PadPackets[0].buttons&8)return 16;
    key.type=SDL_EVENT_KEY_UP;SDL_PushEvent(&key);
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
