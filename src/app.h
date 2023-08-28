//
//  app.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#ifndef app_h
#define app_h

#include <stdio.h>
#include <math.h>

#include "pd_api.h"
#include "scene.h"
#include "utility.h"

typedef struct PGB_Application {
    float dt;
    float crankChange;
    PGB_Scene *scene;
    PGB_Scene *pendingScene;
    LCDFont *bodyFont;
    LCDFont *titleFont;
    LCDFont *subheadFont;
    LCDFont *labelFont;
} PGB_Application;

extern PGB_Application *PGB_App;

void PGB_init(void);
void PGB_update(float dt);
void PGB_present(PGB_Scene *scene);
void PGB_quit(void);

#endif /* app_h */
