//
//  preferences.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 18/05/22.
//

#include "preferences.h"

static const int pref_version = 2;

static const char *pref_filename = "preferences.bin";
static SDFile *pref_file;

bool preferences_sound_enabled = false;
bool preferences_display_fps = false;
bool preferences_frame_skip = false;

static void cpu_endian_to_big_endian(unsigned char *src, unsigned char *buffer, size_t size, size_t len);

static uint8_t prefereces_read_uint8(void);
static void prefereces_write_uint8(uint8_t value);
static uint32_t prefereces_read_uint32(void);
static void prefereces_write_uint32(uint32_t value);

void prefereces_init(void)
{
    preferences_sound_enabled = false;
    if(pgb_get_hardware_rev() == PGB_HardwareRevB)
    {
        preferences_sound_enabled = true;
    }
    preferences_display_fps = false;
    preferences_frame_skip = true;
    
    if(playdate->file->stat(pref_filename, NULL) != 0)
    {
        prefereces_save_to_disk();
    }
    else
    {
        prefereces_read_from_disk();
    }
}

void prefereces_read_from_disk(void)
{
    pref_file = playdate->file->open(pref_filename, kFileReadData);
    if(pref_file)
    {
        // read model version
        uint32_t version = prefereces_read_uint32();
        
        preferences_sound_enabled = prefereces_read_uint8();
        preferences_display_fps = prefereces_read_uint8();
        
        if(version >= 2)
        {
            preferences_frame_skip = prefereces_read_uint8();
        }
        
        playdate->file->close(pref_file);
    }
}

void prefereces_save_to_disk(void)
{
    pref_file = playdate->file->open(pref_filename, kFileWrite);
    
    prefereces_write_uint32(pref_version);
    
    prefereces_write_uint8(preferences_sound_enabled ? 1 : 0);
    prefereces_write_uint8(preferences_display_fps ? 1 : 0);
    prefereces_write_uint8(preferences_frame_skip ? 1 : 0);

    playdate->file->close(pref_file);
}

static uint8_t prefereces_read_uint8(void)
{
    uint8_t buffer[1];
    playdate->file->read(pref_file, buffer, sizeof(uint8_t));
    return buffer[0];
}

static void prefereces_write_uint8(uint8_t value)
{
    playdate->file->write(pref_file, &value, sizeof(uint8_t));
}

static uint32_t prefereces_read_uint32(void)
{
    unsigned char buffer[sizeof(uint32_t)];
    playdate->file->read(pref_file, buffer, sizeof(uint32_t));
    return buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
}

static void prefereces_write_uint32(uint32_t value)
{
    unsigned char buffer[sizeof(uint32_t)];
    cpu_endian_to_big_endian((unsigned char*)&value, buffer, sizeof(uint32_t), 1);
    playdate->file->write(pref_file, buffer, sizeof(uint32_t));
}

static void cpu_endian_to_big_endian(unsigned char *src, unsigned char *buffer, size_t size, size_t len)
{
    int x = 1;
    
    if(*((char*)&x) == 1)
    {
        // little endian machine, swap
        for(size_t i = 0; i < len; i++)
        {
            for (size_t ix = 0; ix < size; ix++)
            {
                buffer[size * i + ix] = src[size * i + (size - 1 - ix)];
            }
        }
    }
    else
    {
        memcpy(buffer, src, size * len);
    }
}
