//
//  utility.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#ifndef utility_h
#define utility_h

#include <stdio.h>
#include <stdbool.h>
#include "pd_api.h"

extern PlaydateAPI *playdate;

#define PGB_DEBUG 0
#define PGB_DEBUG_UPDATED_ROWS 0

#define PGB_LCD_WIDTH 320
#define PGB_LCD_HEIGHT 240
#define PGB_LCD_ROWSIZE 40

#define PGB_LCD_X 32 // multiple of 8
#define PGB_LCD_Y 0

typedef enum {
    PGB_HardwareRevUnknown,
    PGB_HardwareRevA,
    PGB_HardwareRevB
} PGB_HardwareRev;

extern const uint8_t PGB_patterns[4][4][4];

extern const char *PGB_savesPath;
extern const char *PGB_gamesPath;

char* string_copy(const char *string);

static inline int pgb_min(const int a, const int b)
{
    return a < b ? a : b;
}

static inline int pgb_max(const int a, const int b)
{
    return a > b ? a : b;
}

char* pgb_save_filename(const char *filename, bool isRecovery);
char* pgb_extract_fs_error_code(const char *filename);
PGB_HardwareRev pgb_get_hardware_rev(void);

float pgb_easeInOutQuad(float x);

void pgb_fillRoundRect(PDRect rect, int radius, LCDColor color);
void pgb_drawRoundRect(PDRect rect, int radius, int lineWidth, LCDColor color);

void* pgb_malloc(size_t size);
void* pgb_realloc(void *ptr, size_t size);
void* pgb_calloc(size_t count, size_t size);
void pgb_free(void *ptr);

#endif /* utility_h */
