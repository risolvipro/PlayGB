//
//  listview.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 16/05/22.
//

#include "listview.h"
#include "app.h"

static PGB_ListItem* PGB_ListItem_new(void);
static void PGB_ListView_selectItem(PGB_ListView *listView, unsigned int index);
static void PGB_ListItem_super_free(PGB_ListItem *item);

static int PGB_ListView_rowHeight = 32;
static int PGB_ListView_inset = 14;

PGB_ListView* PGB_ListView_new(void) {
    PGB_ListView *listView = pgb_malloc(sizeof(PGB_ListView));
    listView->items = array_new();
    listView->frame = PDRectMake(0, 0, 200, 200);
    
    listView->contentSize = 0;
    listView->contentOffset = 0;
    
    listView->selectedItem = -1;
    
    listView->model = (PGB_ListViewModel){
        .selectedItem = -1,
        .contentOffset = 0,
        .empty = true
    };
    
    return listView;
}

void PGB_ListView_invalidateLayout(PGB_ListView *listView){
    
    int y = 0;
    
    for(int i = 0; i < listView->items->length; i++){
        PGB_ListItem *item = listView->items->items[i];
        item->offsetY = y;
        y += item->height;
    }
    
    listView->contentSize = y;
}

void PGB_ListView_reload(PGB_ListView *listView){
    
    PGB_ListView_invalidateLayout(listView);
    
    int numberOfItems = listView->items->length;
    
    if(numberOfItems > 0){
        if(listView->selectedItem < 0){
            PGB_ListView_selectItem(listView, 0);
        }
        else if(listView->selectedItem >= numberOfItems){
            PGB_ListView_selectItem(listView, numberOfItems - 1);
        }
    }
    else {
        listView->contentOffset = 0;
        listView->selectedItem = -1;
    }
    
    listView->needsDisplay = true;
}

void PGB_ListView_update(PGB_ListView *listView){
    
    PDButtons released;
    playdate->system->getButtonState(NULL, NULL, &released);
    
    if(released & kButtonDown){
        int nextIndex = listView->selectedItem + 1;
        if(nextIndex >= 0 && nextIndex < listView->items->length){
            PGB_ListView_selectItem(listView, nextIndex);
        }
    }
    else if(released & kButtonUp){
        int prevIndex = listView->selectedItem - 1;
        if(prevIndex >= 0 && prevIndex < listView->items->length){
            PGB_ListView_selectItem(listView, prevIndex);
        }
    }
}

void PGB_ListView_draw(PGB_ListView *listView){
    
    bool needsDisplay = false;
    
    if(listView->model.empty || listView->needsDisplay || listView->model.selectedItem != listView->selectedItem || listView->model.contentOffset != listView->contentOffset){
        
        needsDisplay = true;
    }
    
    listView->needsDisplay = false;
    
    listView->model.empty = false;
    listView->model.selectedItem = listView->selectedItem;
    listView->model.contentOffset = listView->contentOffset;
    
    if(needsDisplay){
        
        int listX = listView->frame.x;
        int listY = listView->frame.y;

        playdate->graphics->fillRect(listX, listY, listView->frame.width, listView->frame.height, kColorWhite);
                
        for(int i = 0; i < listView->items->length; i++){
            PGB_ListItem *item = listView->items->items[i];
            
            int rowY = listY + item->offsetY - listView->contentOffset;
            
            bool selected = (i == listView->selectedItem);
            
            if(selected){
                playdate->graphics->fillRect(listX, rowY, listView->frame.width, item->height, kColorBlack);
            }
            
            if(item->type == PGB_ListViewItemTypeButton){
                PGB_ListItemButton *itemButton = item->object;
                
                if(selected){
                    playdate->graphics->setDrawMode(kDrawModeFillWhite);
                }
                else {
                    playdate->graphics->setDrawMode(kDrawModeFillBlack);
                }
                
                int textX = listX + PGB_ListView_inset;
                int textY = rowY + (float)(item->height - playdate->graphics->getFontHeight(PGB_App->subheadFont)) / 2;
                
                playdate->graphics->setFont(PGB_App->subheadFont);
                playdate->graphics->drawText(itemButton->title, strlen(itemButton->title), kUTF8Encoding, textX, textY);
                
                playdate->graphics->setDrawMode(kDrawModeCopy);
            }
        }
    }
}

void PGB_ListView_selectItem(PGB_ListView *listView, unsigned int index){
    
    PGB_ListItem *item = listView->items->items[index];
    
    int listHeight = playdate->display->getHeight();
    
    int centeredOffset = 0;
    
    if(listView->contentSize > listHeight){
        centeredOffset = item->offsetY - ((float)listHeight / 2 - (float)PGB_ListView_rowHeight / 2);
        centeredOffset = PGB_MAX(0, centeredOffset);
        centeredOffset = PGB_MIN(centeredOffset, listView->contentSize - listHeight);
    }
    
    listView->contentOffset = centeredOffset;
    listView->selectedItem = index;
}

void PGB_ListView_free(PGB_ListView *listView){
    
    array_free(listView->items);
    pgb_free(listView);
}

PGB_ListItem* PGB_ListItem_new(void) {
    PGB_ListItem *item = pgb_malloc(sizeof(PGB_ListItem));
    return item;
}

PGB_ListItemButton* PGB_ListItemButton_new(char *title) {
    
    PGB_ListItem *item = PGB_ListItem_new();
    
    PGB_ListItemButton *buttonItem = pgb_malloc(sizeof(PGB_ListItemButton));
    buttonItem->item = item;
    
    item->type = PGB_ListViewItemTypeButton;
    item->object = buttonItem;
    
    item->height = PGB_ListView_rowHeight;
    
    buttonItem->title = string_copy(title);
    
    return buttonItem;
}

void PGB_ListItem_super_free(PGB_ListItem *item){
    
    pgb_free(item);
}

void PGB_ListItemButton_free(PGB_ListItemButton *itemButton){

    PGB_ListItem_super_free(itemButton->item);
    
    pgb_free(itemButton->title);
    pgb_free(itemButton);
}

void PGB_ListItem_free(PGB_ListItem *item){
    
    if(item->type == PGB_ListViewItemTypeButton){
        PGB_ListItemButton_free(item->object);
    }
}
