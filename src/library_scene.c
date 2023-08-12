//
//  library_scene.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 15/05/22.
//

#include "library_scene.h"
#include "app.h"
#include "game_scene.h"
#include "preferences.h"

static void PGB_LibraryScene_update(void *object);
static void PGB_LibraryScene_free(void *object);
static void PGB_LibraryScene_reloadList(PGB_LibraryScene *libraryScene);
static void PGB_LibraryScene_menu(void *object);

static PDMenuItem *audioMenuItem;
static PDMenuItem *fpsMenuItem;

PGB_LibraryScene* PGB_LibraryScene_new(void) {
    
    PGB_Scene *scene = PGB_Scene_new();
    
    PGB_LibraryScene *libraryScene = pgb_malloc(sizeof(PGB_LibraryScene));
    libraryScene->scene = scene;
    
    scene->managedObject = libraryScene;
    
    scene->update = PGB_LibraryScene_update;
    scene->free = PGB_LibraryScene_free;
    scene->menu = PGB_LibraryScene_menu;
    
    libraryScene->model = (PGB_LibrarySceneModel){
        .empty = true,
        .tab = PGB_LibrarySceneTabList
    };

    libraryScene->firstLoad = false;
    
    libraryScene->games = array_new();
    libraryScene->listView = PGB_ListView_new();
    libraryScene->tab = PGB_LibrarySceneTabList;
    
    return libraryScene;
}

void PGB_LibraryScene_listFiles(const char *filename, void *userdata) {
    
    PGB_LibraryScene *libraryScene = userdata;
    
    char *extension;
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename){
        extension = "";
    }
    else {
        extension = dot + 1;
    }
    
    if((strcmp(extension, "gb") == 0 || strcmp(extension, "gbc") == 0)){
        PGB_Game *game = PGB_Game_new(filename);
        array_push(libraryScene->games, game);
    }
}

void PGB_LibraryScene_reloadList(PGB_LibraryScene *libraryScene) {
    
    for(int i = 0; i < libraryScene->games->length; i++){
        PGB_Game *game = libraryScene->games->items[i];
        PGB_Game_free(game);
    }
    
    array_clear(libraryScene->games);
    
    playdate->file->listfiles(PGB_gamesPath, PGB_LibraryScene_listFiles, libraryScene, 0);
    
    PGB_Array *items = libraryScene->listView->items;
    
    for(int i = 0; i < items->length; i++){
        PGB_ListItem *item = items->items[i];
        PGB_ListItem_free(item);
    }
    
    array_clear(items);
    
    for(int i = 0; i < libraryScene->games->length; i++){
        PGB_Game *game = libraryScene->games->items[i];
        
        PGB_ListItemButton *itemButton = PGB_ListItemButton_new(game->filename);
        array_push(items, itemButton->item);
    }
    
    if(items->length > 0){
        libraryScene->tab = PGB_LibrarySceneTabList;
    }
    else {
        libraryScene->tab = PGB_LibrarySceneTabEmpty;
    }
    
    PGB_ListView_reload(libraryScene->listView);
}

void PGB_LibraryScene_update(void *object) {
    
    PGB_LibraryScene *libraryScene = object;
    
    PGB_Scene_update(libraryScene->scene);
    
    if(!libraryScene->firstLoad){
        libraryScene->firstLoad = true;
        PGB_LibraryScene_reloadList(libraryScene);
    }
    
    PDButtons released;
    playdate->system->getButtonState(NULL, NULL, &released);

    if(released & kButtonA){
        int selectedItem = libraryScene->listView->selectedItem;
        if(selectedItem >= 0 && selectedItem < libraryScene->listView->items->length){
            
            PGB_Game *game = libraryScene->games->items[selectedItem];
            
            PGB_GameScene *gameScene = PGB_GameScene_new(game->fullpath);
            PGB_present(gameScene->scene);
        }
    }
    
    bool needsDisplay = false;
    
    if(libraryScene->model.empty || libraryScene->model.tab != libraryScene->tab){
        needsDisplay = true;
    }
    
    libraryScene->model.empty = false;
    libraryScene->model.tab = libraryScene->tab;
    
    if(needsDisplay){
        playdate->graphics->clear(kColorWhite);
    }
    
    if(libraryScene->tab == PGB_LibrarySceneTabList){
        
        libraryScene->listView->needsDisplay = needsDisplay;
        libraryScene->listView->frame = PDRectMake(0, 0, playdate->display->getWidth(), playdate->display->getHeight());
        
        PGB_ListView_update(libraryScene->listView);
        PGB_ListView_draw(libraryScene->listView);
    }
    else if(libraryScene->tab == PGB_LibrarySceneTabEmpty){
        
        if(needsDisplay){
            
            static const char *title = "PlayGB";
            static const char *message1 = "Connect to a computer and";
            static const char *message2 = "copy games to Data/*.playgb/games";
            
            playdate->graphics->clear(kColorWhite);
            
            int titleToMessageSpacing = 6;
            
            int titleHeight = playdate->graphics->getFontHeight(PGB_App->titleFont);
            int messageHeight = playdate->graphics->getFontHeight(PGB_App->bodyFont);
            int messageLineSpacing = 2;
            
            int containerHeight = titleHeight + titleToMessageSpacing + messageHeight * 2 + messageLineSpacing;
            int titleY = (float)(playdate->display->getHeight() - containerHeight) / 2;
            
            int titleX = (float)(playdate->display->getWidth() - playdate->graphics->getTextWidth(PGB_App->titleFont, title, strlen(title), kUTF8Encoding, 0)) / 2;
            int message1_X = (float)(playdate->display->getWidth() - playdate->graphics->getTextWidth(PGB_App->bodyFont, message1, strlen(message1), kUTF8Encoding, 0)) / 2;
            int message2_X = (float)(playdate->display->getWidth() - playdate->graphics->getTextWidth(PGB_App->bodyFont, message2, strlen(message2), kUTF8Encoding, 0)) / 2;
            
            int message1_Y = titleY + titleHeight + titleToMessageSpacing;
            int message2_Y = message1_Y + messageHeight + messageLineSpacing;
            
            playdate->graphics->setFont(PGB_App->titleFont);
            playdate->graphics->drawText(title, strlen(title), kUTF8Encoding, titleX, titleY);
            
            playdate->graphics->setFont(PGB_App->bodyFont);
            playdate->graphics->drawText(message1, strlen(message1), kUTF8Encoding, message1_X, message1_Y);
            playdate->graphics->drawText(message2, strlen(message2), kUTF8Encoding, message2_X, message2_Y);
        }
    }
}

void PGB_LibraryScene_didSelectRefresh(void *userdata){
    
    PGB_LibraryScene *libraryScene = userdata;
    PGB_LibraryScene_reloadList(libraryScene);
}

void PGB_LibraryScene_didChangeSound(void *userdata){
    
    preferences_sound_enabled = playdate->system->getMenuItemValue(audioMenuItem);
}

void PGB_LibraryScene_didChangeFPS(void *userdata){
    
    preferences_display_fps = playdate->system->getMenuItemValue(fpsMenuItem);
}

void PGB_LibraryScene_menu(void *object) {
    
    PGB_LibraryScene *libraryScene = object;
    
    // playdate->system->addMenuItem("Refresh", PGB_LibraryScene_didSelectRefresh, libraryScene);
    
    audioMenuItem = playdate->system->addCheckmarkMenuItem("Sound", preferences_sound_enabled, PGB_LibraryScene_didChangeSound, libraryScene);
    fpsMenuItem = playdate->system->addCheckmarkMenuItem("Show FPS", preferences_display_fps, PGB_LibraryScene_didChangeFPS, libraryScene);
}

void PGB_LibraryScene_free(void *object) {
    
    PGB_LibraryScene *libraryScene = object;
    
    PGB_Scene_free(libraryScene->scene);
    
    for(int i = 0; i < libraryScene->games->length; i++){
        PGB_Game *game = libraryScene->games->items[i];
        PGB_Game_free(game);
    }
    
    PGB_Array *items = libraryScene->listView->items;
    
    for(int i = 0; i < items->length; i++){
        PGB_ListItem *item = items->items[i];
        PGB_ListItem_free(item);
    }
    
    PGB_ListView_free(libraryScene->listView);
    
    array_free(libraryScene->games);
    
    pgb_free(libraryScene);
}

PGB_Game* PGB_Game_new(const char *filename) {
    PGB_Game *game = pgb_malloc(sizeof(PGB_Game));
    game->filename = string_copy(filename);
    
    char *dir_sep = "/";
    
    char *fullpath = pgb_malloc((strlen(PGB_gamesPath) + strlen(dir_sep) + strlen(filename) + 1) * sizeof(char));
    
    strcpy(fullpath, "");
    strcat(fullpath, PGB_gamesPath);
    strcat(fullpath, dir_sep);
    strcat(fullpath, filename);
    
    game->fullpath = fullpath;
    
    return game;
}

void PGB_Game_free(PGB_Game *game) {
    
    pgb_free(game->filename);
    pgb_free(game->fullpath);
    
    pgb_free(game);
}
