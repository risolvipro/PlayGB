//
//  game_scene.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#ifndef game_scene_h
#define game_scene_h

#include <math.h>
#include <stdio.h>

#include "scene.h"

typedef struct PGB_GameSceneContext PGB_GameSceneContext;
typedef struct PGB_GameScene PGB_GameScene;

extern PGB_GameScene *audioGameScene;

typedef enum
{
    PGB_GameSceneStateLoaded,
    PGB_GameSceneStateError
} PGB_GameSceneState;

typedef enum
{
    PGB_GameSceneErrorUndefined,
    PGB_GameSceneErrorLoadingRom,
    PGB_GameSceneErrorWrongLocation,
    PGB_GameSceneErrorFatal
} PGB_GameSceneError;

typedef struct
{
    PGB_GameSceneState state;
    PGB_GameSceneError error;
    int selectorToggleY;
    bool selectorStartPressed;
    bool selectorSelectPressed;
    bool empty;
} PGB_GameSceneModel;

typedef struct
{
    int width;
    int height;
    int containerWidth;
    int containerHeight;
    int containerX;
    int containerY;
    int toggleSize;
    int x;
    int y;
    int startButtonX;
    int startButtonY;
    int selectButtonX;
    int selectButtonY;
    float triggerAngle;
    float deadAngle;
    float toggleY;
    bool startPressed;
    bool selectPressed;
} PGB_CrankSelector;

typedef struct PGB_GameScene
{
    PGB_Scene *scene;
    char *save_filename;
    char *rom_filename;

    bool needsDisplay;
    bool audioEnabled;
    bool audioLocked;
    unsigned int rtc_time;

    PGB_GameSceneState state;
    PGB_GameSceneContext *context;
    PGB_GameSceneModel model;
    PGB_GameSceneError error;

    PGB_CrankSelector selector;

#if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
    PDRect debug_highlightFrame;
    bool debug_updatedRows[LCD_ROWS];
#endif
} PGB_GameScene;

PGB_GameScene *PGB_GameScene_new(const char *rom_filename);

#endif /* game_scene_h */
