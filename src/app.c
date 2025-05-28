//
//  app.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#include "app.h"
#include "game_scene.h"
#include "library_scene.h"
#include "preferences.h"
#include "minigb_apu.h"

PGB_Application *PGB_App;

void PGB_init(void)
{
    PGB_App = pgb_malloc(sizeof(PGB_Application));
    
    PGB_App->scene = NULL;
    PGB_App->pendingScene = NULL;
    
    playdate->file->mkdir("games");
    playdate->file->mkdir("saves");
    
    prefereces_init();
    
    PGB_App->bodyFont = playdate->graphics->loadFont("fonts/Roobert-11-Medium", NULL);
    PGB_App->titleFont = playdate->graphics->loadFont("fonts/Roobert-20-Medium", NULL);
    PGB_App->subheadFont = playdate->graphics->loadFont("fonts/Asheville-Sans-14-Bold", NULL);
    PGB_App->labelFont = playdate->graphics->loadFont("fonts/Nontendo-Bold", NULL);
    
    PGB_App->selectorBitmapTable = playdate->graphics->loadBitmapTable("images/selector/selector", NULL);
    PGB_App->selectorBackground = playdate->graphics->getTableBitmap(PGB_App->selectorBitmapTable, 0);
    PGB_App->selectorButton = playdate->graphics->getTableBitmap(PGB_App->selectorBitmapTable, 1);
    PGB_App->selectorFilledButton = playdate->graphics->getTableBitmap(PGB_App->selectorBitmapTable, 2);

    // add audio callback
    PGB_App->soundSource = playdate->sound->addSource(audio_callback, &audioGameScene, 1);
    
    // custom frame rate delimiter
    playdate->display->setRefreshRate(0);
    
    PGB_LibraryScene *libraryScene = PGB_LibraryScene_new();
    PGB_present(libraryScene->scene);
}

void PGB_update(float dt)
{
    PGB_App->dt = dt;
    PGB_App->crankChange = playdate->system->getCrankChange();
    
    if(PGB_App->scene)
    {
        void *managedObject = PGB_App->scene->managedObject;
        PGB_App->scene->update(managedObject);
    }
    
    if(PGB_App->pendingScene)
    {
        // present pending scene
        
        if(PGB_App->scene)
        {
            prefereces_save_to_disk();
            
            void *managedObject = PGB_App->scene->managedObject;
            PGB_App->scene->free(managedObject);
        }
        
        PGB_App->scene = PGB_App->pendingScene;
        PGB_App->pendingScene = NULL;
        PGB_Scene_refreshMenu(PGB_App->scene);
    }
    
    #if PGB_DEBUG
    playdate->display->setRefreshRate(60);
    #else
    
    float refreshRate = 30;
    float compensation = 0;
    
    if(PGB_App->scene)
    {
        refreshRate = PGB_App->scene->preferredRefreshRate;
        compensation = PGB_App->scene->refreshRateCompensation;
    }
    
    if(refreshRate > 0)
    {
        float refreshInterval = 1.0f / refreshRate + compensation;
        while(playdate->system->getElapsedTime() < refreshInterval);
    }
    
    #endif
}

void PGB_present(PGB_Scene *scene)
{
    PGB_App->pendingScene = scene;
}

void PGB_quit(void)
{
    prefereces_save_to_disk();
    
    if(PGB_App->scene)
    {
        void *managedObject = PGB_App->scene->managedObject;
        PGB_App->scene->free(managedObject);
    }
}
