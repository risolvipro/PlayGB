//
//  utility.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#include "utility.h"

PlaydateAPI *playdate;

const char *PGB_savesPath = "saves";
const char *PGB_gamesPath = "games";

const uint8_t PGB_patterns[4][4][4] = {
    {
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1}
    },
    {
        {0, 1, 0, 1},
        {1, 1, 1, 1},
        {0, 1, 0, 1},
        {1, 1, 1, 1}
    },
    {
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }
};

char* string_copy(const char *string){
    char *copied = pgb_malloc((strlen(string) + 1) * sizeof(char));
    strcpy(copied, string);
    return copied;
}

char* pgb_save_filename(const char *path, bool isRecovery){
    
    char *filename;
    
    char *slash = strrchr(path, '/');
    if(!slash){
        filename = (char*)path;
    }
    else {
        filename = slash + 1;
    }
    
    size_t len;
    
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename){
        len = strlen(filename);
    }
    else {
        len = strlen(filename) - strlen(dot);
    }
    
    char *dir_sep = "/";
    char *extension = ".sav";
    
    char *recoverySuffix = " (recovery)";
    
    size_t recovery_len = 0;
    if(isRecovery){
        recovery_len = strlen(recoverySuffix);
    }
    
    char *buffer = pgb_malloc((strlen(PGB_savesPath) + strlen(dir_sep) + len + recovery_len + strlen(extension) + 1) * sizeof(char));
    
    strcpy(buffer, "");
    
    strcat(buffer, PGB_savesPath);
    strcat(buffer, dir_sep);
    strncat(buffer, filename, len);
    if(isRecovery){
        strcat(buffer, recoverySuffix);
    }
    strcat(buffer, extension);
    
    return buffer;
}

void* pgb_malloc(size_t size) {
    return playdate->system->realloc(NULL, size);
}

void* pgb_realloc(void *ptr, size_t size) {
    return playdate->system->realloc(ptr, size);
}

void* pgb_calloc(size_t count, size_t size) {
    return memset(pgb_malloc(count * size), 0, count * size);
}

void pgb_free(void *ptr) {
    playdate->system->realloc(ptr, 0);
}
