//
//  preferences.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 18/05/22.
//

#ifndef preferences_h
#define preferences_h

#include <stdio.h>
#include "utility.h"

extern bool preferences_sound_enabled;
extern bool preferences_display_fps;

void prefereces_init(void);

void prefereces_read_from_disk(void);
void prefereces_save_to_disk(void);

#endif /* preferences_h */
