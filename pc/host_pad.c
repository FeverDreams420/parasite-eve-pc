#include "host_pad.h"
#include "pe1/pad_state.h"
#include <string.h>

static bool held[SDL_SCANCODE_COUNT];
static bool focused;
static void publish(void) {
    unsigned buttons=0;
    if (focused) {
        if(held[SDL_SCANCODE_UP] || held[SDL_SCANCODE_W])buttons|=0x10;
        if(held[SDL_SCANCODE_RIGHT] || held[SDL_SCANCODE_D])buttons|=0x20;
        if(held[SDL_SCANCODE_DOWN] || held[SDL_SCANCODE_S])buttons|=0x40;
        if(held[SDL_SCANCODE_LEFT] || held[SDL_SCANCODE_A])buttons|=0x80;
        if(held[SDL_SCANCODE_LSHIFT] || held[SDL_SCANCODE_RSHIFT] || held[SDL_SCANCODE_Z])buttons|=0x2000;
        if(held[SDL_SCANCODE_X] || held[SDL_SCANCODE_SPACE])buttons|=0x4000;
        if(held[SDL_SCANCODE_C])buttons|=0x1000;
        if(held[SDL_SCANCODE_V])buttons|=0x8000;
        if(held[SDL_SCANCODE_RETURN] || held[SDL_SCANCODE_ESCAPE])buttons|=8;
        if(held[SDL_SCANCODE_TAB])buttons|=1;
        if(held[SDL_SCANCODE_Q])buttons|=0x400;
        if(held[SDL_SCANCODE_E])buttons|=0x800;
    }
    g_PadPackets[0].header=0x4100;
    g_PadPackets[0].buttons=(u16)~buttons;
    g_PadPackets[0].leftX=g_PadPackets[0].leftY=128;
    g_PadPackets[0].rightX=g_PadPackets[0].rightY=128;
}
void pe_host_pad_reset(void) {
    memset(held,0,sizeof(held));focused=true;
    memset(g_PadPackets,0,sizeof(g_PadPackets));
    g_PadPackets[1].header=0x00ff;
    publish();
}
void pe_host_pad_event(const SDL_Event *event) {
    if(event->type==SDL_EVENT_WINDOW_FOCUS_LOST || event->type==SDL_EVENT_QUIT) {
        focused=false;memset(held,0,sizeof(held));
    } else if(event->type==SDL_EVENT_WINDOW_FOCUS_GAINED)focused=true;
    else if(focused && (event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_KEY_UP)) {
        SDL_Scancode key=event->key.scancode;
        if(key>SDL_SCANCODE_UNKNOWN && key<SDL_SCANCODE_COUNT)
            held[key]=event->type==SDL_EVENT_KEY_DOWN;
    }
    publish();
}
