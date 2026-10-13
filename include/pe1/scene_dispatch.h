#ifndef PE1_SCENE_DISPATCH_H
#define PE1_SCENE_DISPATCH_H

/* Dispatch token of the scene that is running; the main loop sets it and the
 * map selection compares it with g_SceneDispatchToken. */
extern int g_SceneDispatchCur;

/* Packed id of the room to load next (0x8009D280); the save tail stores it. */
extern int g_SceneDispatchToken;


#endif
