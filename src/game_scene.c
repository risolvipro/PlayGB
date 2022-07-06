//
//  game_scene.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 14/05/22.
//

#include "game_scene.h"
#include "minigb_apu.h"
#include "peanut_gb.h"
#include "app.h"
#include "library_scene.h"
#include "preferences.h"

typedef struct PGB_GameSceneContext {
    PGB_GameScene *scene;
    struct gb_s gb;
    uint8_t *rom;
    uint8_t *cart_ram;
} PGB_GameSceneContext;

static void PGB_GameScene_selector_init(PGB_GameScene *gameScene);
static void PGB_GameScene_update(void *object);
static void PGB_GameScene_menu(void *object);
static void PGB_GameScene_saveGame(PGB_GameScene *gameScene);
static void PGB_GameScene_refreshMenu(PGB_GameScene *gameScene);
static void PGB_GameScene_free(void *object);

static uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr);
static uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr);
static void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val);

static uint8_t *read_rom_to_ram(const char *filename);

static void read_cart_ram_file(const char *save_filename, uint8_t **dest, const size_t len);
static void write_cart_ram_file(const char *save_filename, uint8_t **dest, const size_t len);

static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val);

static const char *startButtonText = "start";
static const char *selectButtonText = "select";

PGB_GameScene* PGB_GameScene_new(const char *rom_filename){
    
    PGB_Scene *scene = PGB_Scene_new();
    
    PGB_GameScene *gameScene = pgb_malloc(sizeof(PGB_GameScene));
    gameScene->scene = scene;
    
    scene->managedObject = gameScene;
    
    scene->update = PGB_GameScene_update;
    scene->menu = PGB_GameScene_menu;
    scene->free = PGB_GameScene_free;

    scene->preferredFrameRate = 60;

    gameScene->rtc_timer = 0;
    
    gameScene->rom_filename = string_copy(rom_filename);
    gameScene->save_filename = NULL;

    gameScene->state = PGB_GameSceneStateError;
    gameScene->error = PGB_GameSceneErrorUndefined;

    gameScene->model = (PGB_GameSceneModel){
        .state = PGB_GameSceneStateError,
        .error = PGB_GameSceneErrorUndefined,
        .selectorIndex = 0,
        .empty = true
    };
    
    gameScene->needsDisplay = false;
    
    gameScene->soundSource = NULL;
    
    gameScene->audioEnabled = preferences_sound_enabled;
    gameScene->audioLocked = false;

    PGB_GameScene_selector_init(gameScene);
    
    #if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
    int highlightWidth = 10;
    gameScene->debug_highlightFrame = PDRectMake(PGB_LCD_X - 1 - highlightWidth, 0, highlightWidth, playdate->display->getHeight());
    #endif
    
    PGB_GameSceneContext *context = pgb_malloc(sizeof(PGB_GameSceneContext));
    gameScene->context = context;
    
    context->scene = gameScene;
    context->gb = (struct gb_s){};
    context->rom = NULL;
    context->cart_ram = NULL;
    
    uint8_t *rom = read_rom_to_ram(rom_filename);
    if(rom != NULL){
        context->rom = rom;
        
        enum gb_init_error_e gb_ret = gb_init(&context->gb, gb_rom_read, gb_cart_ram_read, gb_cart_ram_write, gb_error, context);
        
        if(gb_ret == GB_INIT_NO_ERROR){
            char *save_filename = pgb_save_filename(rom_filename, false);
            gameScene->save_filename = save_filename;
            
            read_cart_ram_file(save_filename, &context->cart_ram, gb_get_save_size(&context->gb));
            
            if(gameScene->audioEnabled){
                // init audio
                gameScene->soundSource = playdate->sound->addSource(audio_callback, gameScene, 1);
                playdate->sound->channel->setVolume(playdate->sound->getDefaultChannel(), 0.2f);
                
                audio_init();
                
                context->gb.direct.sound = 1;
            }
            
            // init lcd
            gb_init_lcd(&context->gb);
            
            context->gb.direct.frame_skip = 1;

            // set game state to loaded
            gameScene->state = PGB_GameSceneStateLoaded;
        }
        else {
            gameScene->state = PGB_GameSceneStateError;
            gameScene->error = PGB_GameSceneErrorFatal;
            
            playdate->system->logToConsole("%s:%i: Error initializing gb context", __FILE__, __LINE__);
        }
    }
    else {
        gameScene->state = PGB_GameSceneStateError;
        gameScene->error = PGB_GameSceneErrorLoadingRom;
    }
            
    return gameScene;
}

void PGB_GameScene_selector_init(PGB_GameScene *gameScene){
    
    int startButtonWidth = playdate->graphics->getTextWidth(PGB_App->labelFont, startButtonText, strlen(startButtonText), kUTF8Encoding, 0);
    int selectButtonWidth = playdate->graphics->getTextWidth(PGB_App->labelFont, selectButtonText, strlen(selectButtonText), kUTF8Encoding, 0);
    
    int width = 18;
    int height = 46;
    
    int startSpacing = 3;
    int selectSpacing = 6;

    int labelHeight = playdate->graphics->getFontHeight(PGB_App->labelFont);

    int containerHeight = labelHeight + startSpacing + height + selectSpacing + labelHeight;
    int containerWidth = width;
    
    containerWidth = PGB_MAX(containerWidth, startButtonWidth);
    containerWidth = PGB_MAX(containerWidth, selectButtonWidth);
    
    int containerX = playdate->display->getWidth() - 6 - containerWidth;
    int containerY = 8;
    
    int x = containerX + (float)(containerWidth - width) / 2;
    int y = containerY + labelHeight + startSpacing;
    
    int startButtonX = containerX + (float)(containerWidth - startButtonWidth) / 2;
    int startButtonY = containerY;
    
    int selectButtonX = containerX + (float)(containerWidth - selectButtonWidth) / 2;
    int selectButtonY = containerY + containerHeight - labelHeight;
    
    gameScene->selector = (PGB_CrankSelector){};
    gameScene->selector.x = x;
    gameScene->selector.y = y;
    gameScene->selector.width = width;
    gameScene->selector.height = height;
    gameScene->selector.containerX = containerX;
    gameScene->selector.containerY = containerY;
    gameScene->selector.containerWidth = containerWidth;
    gameScene->selector.containerHeight = containerHeight;
    gameScene->selector.startButtonX = startButtonX;
    gameScene->selector.startButtonY = startButtonY;
    gameScene->selector.selectButtonX = selectButtonX;
    gameScene->selector.selectButtonY = selectButtonY;
    gameScene->selector.numberOfFrames = 27;
    gameScene->selector.bitmapTable = playdate->graphics->loadBitmapTable("images/selector/selector", NULL);
    gameScene->selector.startSelectBitmap = playdate->graphics->loadBitmap("images/selector-start-select", NULL);
    gameScene->selector.triggerAngle = 15;
    gameScene->selector.deadAngle = 20;
    gameScene->selector.index = 0;
    gameScene->selector.startPressed = false;
    gameScene->selector.selectPressed = false;
}

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr) {
    PGB_GameSceneContext *context = gb->direct.priv;
    return context->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr) {
    PGB_GameSceneContext *context = gb->direct.priv;
    return context->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val){
    PGB_GameSceneContext *context = gb->direct.priv;
    context->cart_ram[addr] = val;
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
uint8_t *read_rom_to_ram(const char *filename) {
    SDFile *rom_file = playdate->file->open(filename, kFileReadData);

    if(rom_file == NULL){
        playdate->system->logToConsole("%s:%i: Can't read rom file %s", __FILE__, __LINE__, filename);
        return NULL;
    }
    
    playdate->file->seek(rom_file, 0, SEEK_END);
    
    size_t rom_size = playdate->file->tell(rom_file);
    
    playdate->file->seek(rom_file, 0, SEEK_SET);
    
    uint8_t *rom = pgb_malloc(rom_size);

    size_t len = sizeof(uint8_t) * rom_size;
    
    if(playdate->file->read(rom_file, rom, (unsigned int)len) != rom_size){
        pgb_free(rom);
        playdate->file->close(rom_file);
        return NULL;
    }

    playdate->file->close(rom_file);
    return rom;
}

void read_cart_ram_file(const char *save_filename, uint8_t **dest, const size_t len){

    /* If save file not required. */
    if(len == 0) {
        *dest = NULL;
        return;
    }

    /* Allocate enough memory to hold save file. */
    if((*dest = pgb_malloc(len)) == NULL){
        playdate->system->logToConsole("%s:%i: Error allocating save file %s", __FILE__, __LINE__, save_filename);
    }

    SDFile *f = playdate->file->open(save_filename, kFileReadData);

    /* It doesn't matter if the save file doesn't exist. We initialise the
     * save memory allocated above. The save file will be created on exit. */
    if(f == NULL){
        memset(*dest, 0, len);
        return;
    }

    /* Read save file to allocated memory. */
    playdate->file->read(f, *dest, (unsigned int)(len * sizeof(uint8_t)));
    playdate->file->close(f);
}

void write_cart_ram_file(const char *save_filename, uint8_t **dest, const size_t len){

    if(len == 0 || *dest == NULL){
        return;
    }
    
    SDFile *f = playdate->file->open(save_filename, kFileWrite);
    
    if(f == NULL){
        playdate->system->logToConsole("%s:%i: Can't write save file", __FILE__, __LINE__, save_filename);
        return;
    }

    /* Record save file. */
    playdate->file->write(f, *dest, (unsigned int)(len * sizeof(uint8_t)));
    playdate->file->close(f);
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val) {
    
    PGB_GameSceneContext *context = gb->direct.priv;
    
    bool is_fatal = false;
    
    if(gb_err == GB_INVALID_OPCODE){
        is_fatal = true;
        
        playdate->system->logToConsole("%s:%i: Invalid opcode %#04x at PC: %#06x, SP: %#06x", __FILE__, __LINE__, val, gb->cpu_reg.pc - 1, gb->cpu_reg.sp);
    }
    else if(gb_err == GB_INVALID_READ || gb_err == GB_INVALID_WRITE){
        
        playdate->system->logToConsole("%s:%i: Invalid read / write", __FILE__, __LINE__);
    }
    else {
        is_fatal = true;
        
        playdate->system->logToConsole("%s:%i: Unknown error occurred", __FILE__, __LINE__);
    }
    
    if(is_fatal){
        // write recovery .sav
        char *recovery_filename = pgb_save_filename(context->scene->rom_filename, true);
        write_cart_ram_file(recovery_filename, &context->cart_ram, gb_get_save_size(gb));
        
        pgb_free(recovery_filename);
        
        context->scene->state = PGB_GameSceneStateError;
        context->scene->error = PGB_GameSceneErrorFatal;
        
        PGB_GameScene_refreshMenu(context->scene);
    }

    return;
}

void PGB_GameScene_update(void *object){
    
    PGB_GameScene *gameScene = object;
    
    PGB_Scene_update(gameScene->scene);
            
    float progress = 0.5f;
    
    gameScene->selector.startPressed = false;
    gameScene->selector.selectPressed = false;
    
    if(!playdate->system->isCrankDocked()){
        float angle = fmaxf(0, fminf(360, playdate->system->getCrankAngle()));
        
        if(angle <= (180 - gameScene->selector.deadAngle)){
            if(angle >= gameScene->selector.triggerAngle){
                gameScene->selector.startPressed = true;
            }
            
            float adjustedAngle = fminf(angle, gameScene->selector.triggerAngle);
            progress = 0.5f - adjustedAngle / gameScene->selector.triggerAngle * 0.5f;
        }
        else if(angle >= (180 + gameScene->selector.deadAngle)){
            if(angle <= (360 - gameScene->selector.triggerAngle)){
                gameScene->selector.selectPressed = true;
            }
            
            float adjustedAngle = fminf(360 - angle, gameScene->selector.triggerAngle);
            progress = 0.5f + adjustedAngle / gameScene->selector.triggerAngle * 0.5f;
        }
        else {
            gameScene->selector.startPressed = true;
            gameScene->selector.selectPressed = true;
        }
    }
    
    int selectorIndex;
    
    if(gameScene->selector.startPressed && gameScene->selector.selectPressed){
        selectorIndex = -1;
    }
    else {
        selectorIndex = 1 + roundf(progress * (gameScene->selector.numberOfFrames - 2));
        
        if(progress == 0){
            selectorIndex = 0;
        }
        else if(progress == 1){
            selectorIndex = gameScene->selector.numberOfFrames - 1;
        }
    }
    
    gameScene->selector.index = selectorIndex;
    
    bool needsDisplay = false;
    
    if(gameScene->model.empty || gameScene->needsDisplay || gameScene->model.state != gameScene->state || gameScene->model.error != gameScene->error){
        needsDisplay = true;
    }
    
    bool needsDisplaySelector = false;
    if(needsDisplay || gameScene->model.selectorIndex != gameScene->selector.index){
        needsDisplaySelector = true;
    }

    gameScene->model.empty = false;
    gameScene->model.state = gameScene->state;
    gameScene->model.error = gameScene->error;
    gameScene->model.selectorIndex = gameScene->selector.index;
    gameScene->needsDisplay = false;
    
    if(gameScene->state == PGB_GameSceneStateLoaded){
        PGB_GameSceneContext *context = gameScene->context;
                
        PDButtons current;
        playdate->system->getButtonState(&current, NULL, NULL);
        
        context->gb.direct.joypad_bits.start = !(gameScene->selector.startPressed);
        context->gb.direct.joypad_bits.select = !(gameScene->selector.selectPressed);
           
        context->gb.direct.joypad_bits.a = !(current & kButtonA);
        context->gb.direct.joypad_bits.b = !(current & kButtonB);
        context->gb.direct.joypad_bits.left = !(current & kButtonLeft);
        context->gb.direct.joypad_bits.up = !(current & kButtonUp);
        context->gb.direct.joypad_bits.right = !(current & kButtonRight);
        context->gb.direct.joypad_bits.down = !(current & kButtonDown);
        
        if(needsDisplay){
            playdate->graphics->clear(kColorBlack);
        }
        
        #if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
        memset(gameScene->debug_updatedRows, 0, LCD_ROWS);
        #endif
        
        gb_run_frame(&context->gb);
        
        if(!context->gb.direct.frame_skip || !context->gb.display.frame_skip_count){
            
            uint8_t *framebuffer = playdate->graphics->getFrame();
            
            int skip_counter = 0;
            bool single_line = false;
            
            int y2 = 0;
            int lcd_rows = PGB_LCD_Y * LCD_ROWSIZE + PGB_LCD_X / 8;
            
            int row_offset = LCD_ROWSIZE;
            int row_offset2 = LCD_ROWSIZE * 2;
            
            int y_offset;
            int next_y_offset = 2;
            
            for(int y = 0; y < (LCD_HEIGHT - 1); y++){
                y_offset = next_y_offset;
                
                if(skip_counter == 5){
                    y_offset = 1;
                    next_y_offset = 1;
                    skip_counter = 0;
                    single_line = true;
                }
                else if(single_line){
                    next_y_offset = 2;
                    single_line = false;
                }
                
                if(context->gb.display.changed_lines[y]){
                    uint8_t *pixels = !context->gb.display.back_fb_enabled ? context->gb.display.back_fb[y] : context->gb.display.front_fb[y];
                    
                    int d_row1 = y2 & 3;
                    int d_row2 = (y2 + 1) & 3;
                    
                    int x2 = PGB_LCD_X;
                    
                    int fb_index1 = lcd_rows;
                    int fb_index2 = lcd_rows + row_offset;
                    
                    for(int x = 0; x < LCD_WIDTH; x++){
                        int x3 = x2 + 1;
                        int palette = pixels[x] & 3;
                        
                        int d_col2 = x2 & 3;
                        int d_col3 = x3 & 3;
                        
                        int mask2 = (1 << (7 - (x2 & 7)));
                        int mask3 = (1 << (7 - (x3 & 7)));
                        
                        framebuffer[fb_index1] ^= (-(PGB_patterns[palette][d_row1][d_col2]) ^ framebuffer[fb_index1]) & mask2;
                        framebuffer[fb_index1] ^= (-(PGB_patterns[palette][d_row1][d_col3]) ^ framebuffer[fb_index1]) & mask3;
                        
                        if(y_offset == 2){
                            framebuffer[fb_index2] ^= (-(PGB_patterns[palette][d_row2][d_col2]) ^ framebuffer[fb_index2]) & mask2;
                            framebuffer[fb_index2] ^= (-(PGB_patterns[palette][d_row2][d_col3]) ^ framebuffer[fb_index2]) & mask3;
                        }
                        
                        x2 += 2;
                        
                        if((x2 & 7) == 0){
                            fb_index1 += 1;
                            fb_index2 += 1;
                        }
                    }
                    
                    playdate->graphics->markUpdatedRows(y2, y2 + y_offset - 1);
                        
                    #if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
                    for(int i = 0; i < y_offset; i++){
                        context->scene->debug_updatedRows[y2 + i] = true;
                    }
                    #endif
                }
                
                y2 += y_offset;
                lcd_rows += (y_offset == 1) ? row_offset : row_offset2;
                
                if(!single_line){
                    skip_counter++;
                }
            }
        }
        
        gameScene->rtc_timer += PGB_App->dt;
        
        if(gameScene->rtc_timer >= 1){
            gameScene->rtc_timer = gameScene->rtc_timer - 1;
            gb_tick_rtc(&context->gb);
        }

        if(needsDisplay){
            playdate->graphics->setFont(PGB_App->labelFont);
            playdate->graphics->setDrawMode(kDrawModeFillWhite);
            
            playdate->graphics->drawText(startButtonText, strlen(startButtonText), kUTF8Encoding, gameScene->selector.startButtonX, gameScene->selector.startButtonY);
            playdate->graphics->drawText(selectButtonText, strlen(selectButtonText), kUTF8Encoding, gameScene->selector.selectButtonX, gameScene->selector.selectButtonY);
            
            playdate->graphics->setDrawMode(kDrawModeCopy);
        }
        
        if(needsDisplaySelector){
            LCDBitmap *bitmap;
            
            if(selectorIndex < 0){
                bitmap = gameScene->selector.startSelectBitmap;
            }
            else {
                bitmap = playdate->graphics->getTableBitmap(gameScene->selector.bitmapTable, selectorIndex);
            }
            
            playdate->graphics->drawBitmap(bitmap, gameScene->selector.x, gameScene->selector.y, kBitmapUnflipped);
        }
        
        #if PGB_DEBUG && PGB_DEBUG_UPDATED_ROWS
        PDRect highlightFrame = gameScene->debug_highlightFrame;
        playdate->graphics->fillRect(highlightFrame.x, highlightFrame.y, highlightFrame.width, highlightFrame.height, kColorBlack);
        
        for(int y = 0; y < PGB_LCD_HEIGHT; y++){
            int absoluteY = PGB_LCD_Y + y;
            
            if(gameScene->debug_updatedRows[absoluteY]){
                playdate->graphics->fillRect(highlightFrame.x, absoluteY, highlightFrame.width, 1, kColorWhite);
            }
        }
        #endif
        
        if(preferences_display_fps){
            playdate->system->drawFPS(0, 0);
        }
    }
    else if(gameScene->state == PGB_GameSceneStateError){
        
        if(needsDisplay){
            char *errorTitle = "Oh no!";
            
            char *errorMessage = "A generic error occurred";
            
            if(gameScene->error == PGB_GameSceneErrorLoadingRom){
                errorMessage = "Can't load the selected ROM";
            }
            else if(gameScene->error == PGB_GameSceneErrorFatal){
                errorMessage = "A fatal error occurred";
            }
            
            playdate->graphics->clear(kColorWhite);
            
            int titleToMessageSpacing = 6;
            
            int titleHeight = playdate->graphics->getFontHeight(PGB_App->titleFont);
            int messageHeight = playdate->graphics->getFontHeight(PGB_App->bodyFont);
            
            int containerHeight = titleHeight + titleToMessageSpacing + messageHeight;
            int titleY = (float)(playdate->display->getHeight() - containerHeight) / 2;
            
            int titleX = (float)(playdate->display->getWidth() - playdate->graphics->getTextWidth(PGB_App->titleFont, errorTitle, strlen(errorTitle), kUTF8Encoding, 0)) / 2;
            int messageX = (float)(playdate->display->getWidth() - playdate->graphics->getTextWidth(PGB_App->bodyFont, errorMessage, strlen(errorMessage), kUTF8Encoding, 0)) / 2;
            
            playdate->graphics->setFont(PGB_App->titleFont);
            playdate->graphics->drawText(errorTitle, strlen(errorTitle), kUTF8Encoding, titleX, titleY);
            
            playdate->graphics->setFont(PGB_App->bodyFont);
            playdate->graphics->drawText(errorMessage, strlen(errorMessage), kUTF8Encoding, messageX, titleY + titleHeight + titleToMessageSpacing);
        }
    }
}

void PGB_GameScene_didSelectSave(void *userdata) {
    
    PGB_GameScene *gameScene = userdata;
    
    gameScene->audioLocked = true;
    
    PGB_GameScene_saveGame(gameScene);
    
    gameScene->audioLocked = false;
}

void PGB_GameScene_didSelectLibrary(void *userdata) {
    
    PGB_GameScene *gameScene = userdata;
    
    gameScene->audioLocked = true;
    
    PGB_LibraryScene *libraryScene = PGB_LibraryScene_new();
    PGB_present(libraryScene->scene);
}

void PGB_GameScene_refreshMenu(PGB_GameScene *gameScene) {
    
    playdate->system->removeAllMenuItems();

    playdate->system->addMenuItem("Library", PGB_GameScene_didSelectLibrary, gameScene);
    
    if(gameScene->state == PGB_GameSceneStateLoaded){
        playdate->system->addMenuItem("Save", PGB_GameScene_didSelectSave, gameScene);
    }
}

void PGB_GameScene_menu(void *object) {
    
    PGB_GameScene *gameScene = object;
    
    PGB_GameScene_refreshMenu(gameScene);
}

void PGB_GameScene_saveGame(PGB_GameScene *gameScene) {
    
    if(gameScene->state == PGB_GameSceneStateLoaded){
        PGB_GameSceneContext *context = gameScene->context;
        
        if(gameScene->save_filename != NULL){
            write_cart_ram_file(gameScene->save_filename, &context->cart_ram, gb_get_save_size(&context->gb));
        }
    }
}

void PGB_GameScene_free(void *object){
        
    PGB_GameScene *gameScene = object;
    PGB_GameSceneContext *context = gameScene->context;
    
    PGB_Scene_free(gameScene->scene);
    
    gameScene->audioLocked = true;
    
    if(gameScene->soundSource != NULL){
        playdate->sound->removeSource(gameScene->soundSource);
        pgb_free(gameScene->soundSource);
    }
    
    PGB_GameScene_saveGame(gameScene);
        
    gb_reset(&context->gb);
    
    pgb_free(gameScene->rom_filename);
    
    if(gameScene->save_filename != NULL){
        pgb_free(gameScene->save_filename);
    }
    
    if(context->rom != NULL){
        pgb_free(context->rom);
    }
    
    if(context->cart_ram != NULL){
        pgb_free(context->cart_ram);
    }
    
    pgb_free(context);
    pgb_free(gameScene);
}
