//
//  library_scene.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 15/05/22.
//

#ifndef library_scene_h
#define library_scene_h

#include <stdio.h>

#include "array.h"
#include "listview.h"
#include "scene.h"

typedef enum
{
    PGB_LibrarySceneTabList,
    PGB_LibrarySceneTabEmpty
} PGB_LibrarySceneTab;

typedef struct
{
    bool empty;
    PGB_LibrarySceneTab tab;
} PGB_LibrarySceneModel;

typedef struct
{
    char *filename;
    char *fullpath;
} PGB_Game;

typedef struct PGB_LibraryScene
{
    PGB_Scene *scene;
    PGB_Array *games;
    PGB_LibrarySceneModel model;
    PGB_ListView *listView;
    bool firstLoad;
    PGB_LibrarySceneTab tab;
} PGB_LibraryScene;

PGB_LibraryScene *PGB_LibraryScene_new(void);

PGB_Game *PGB_Game_new(const char *filename);
void PGB_Game_free(PGB_Game *game);

#endif /* library_scene_h */
