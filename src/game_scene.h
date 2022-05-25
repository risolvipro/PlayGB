//
//  game_scene.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#ifndef game_scene_h
#define game_scene_h

#include <stdio.h>
#include <math.h>
#include "scene.h"

typedef enum {
    PGB_GameSceneStateLoaded,
    PGB_GameSceneStateError
} PGB_GameSceneState;

typedef enum {
    PGB_GameSceneErrorUndefined,
    PGB_GameSceneErrorLoadingRom,
    PGB_GameSceneErrorFatal
} PGB_GameSceneError;

typedef struct PGB_GameSceneContext PGB_GameSceneContext;

typedef struct {
    PGB_GameSceneState state;
    PGB_GameSceneError error;
    int selectorIndex;
    bool empty;
} PGB_GameSceneModel;

typedef struct {
    int width;
    int height;
    int containerWidth;
    int containerHeight;
    int containerX;
    int containerY;
    int x;
    int y;
    int startButtonX;
    int startButtonY;
    int selectButtonX;
    int selectButtonY;
    int numberOfFrames;
    float triggerAngle;
    float deadAngle;
    LCDBitmapTable *bitmapTable;
    LCDBitmap *startSelectBitmap;
    float index;
    bool startPressed;
    bool selectPressed;
} PGB_CrankSelector;

typedef struct {
    PGB_Scene *scene;
    char *save_filename;
    char *rom_filename;
    
    bool needsDisplay;
    bool audioEnabled;
    bool audioLocked;
    
    PGB_GameSceneState state;
    PGB_GameSceneContext *context;
    PGB_GameSceneModel model;
    PGB_GameSceneError error;
    
    PGB_CrankSelector selector;
    
    SoundSource *soundSource;
    
    #if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
    PDRect debug_highlightFrame;
    bool debug_updatedRows[LCD_ROWS];
    #endif
    
    float rtc_timer;
} PGB_GameScene;

PGB_GameScene* PGB_GameScene_new(const char *rom_filename);

#endif /* game_scene_h */
