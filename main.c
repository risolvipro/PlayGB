//
//  main.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#include <stdio.h>
#include "pd_api.h"
#include "app.h"

#ifdef _WINDLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

static int update(void* userdata);

DllExport int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t arg) {
    
    if(event == kEventInit){
        playdate = pd;
        
        PGB_init();
        
        pd->system->setUpdateCallback(update, pd);
    }
    else if (event == kEventTerminate){
        PGB_quit();
    }
    
    return 0;
}

int update(void* userdata) {
    PlaydateAPI *pd = userdata;
    
    float dt = pd->system->getElapsedTime();
    pd->system->resetElapsedTime();
    
    PGB_App->dt = dt;
    
    PGB_update();
    
    return 1;
}
