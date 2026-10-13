#ifndef PE1_SCENE_ROOM_LOAD_H
#define PE1_SCENE_ROOM_LOAD_H

/* Room load steps shared by the boot, scene and render code. */
int Render_InitRoomPrimState(void *object);
/* 14-state loader for the area entity texture bank. */
int Scene_LoadEntityTextures(void);
/* Steps the room TIM texture loader. */
int Asset_LoadTimTextures(int force);

/* Model allocation per actor type, given to actors of types below 10. */
extern unsigned int g_SceneMapPrimBaseTable[];

/* Startup resource blob Scene_LoadSceneData copies out of the scratch buffer. */
extern unsigned char g_StartupResourceBlob[];

#endif /* PE1_SCENE_ROOM_LOAD_H */
