//
//  listview.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 16/05/22.
//

#include "listview.h"
#include "app.h"

static PGB_ListItem* PGB_ListItem_new(void);
static void PGB_ListView_selectItem(PGB_ListView *listView, unsigned int index, bool animated);
static void PGB_ListItem_super_free(PGB_ListItem *item);

static int PGB_ListView_rowHeight = 32;
static int PGB_ListView_inset = 14;
static int PGB_ListView_scrollInset = 4;
static int PGB_ListView_scrollIndicatorWidth = 4;
static int PGB_ListView_scrollIndicatorMinHeight = 40;

static float PGB_ListView_repeatInterval1 = 0.15;
static float PGB_ListView_repeatInterval2 = 2;

static float PGB_ListView_crankResetMinTime = 2;
static float PGB_ListView_crankMinChange = 30;

PGB_ListView* PGB_ListView_new(void)
{
    PGB_ListView *listView = pgb_malloc(sizeof(PGB_ListView));
    listView->items = array_new();
    listView->frame = PDRectMake(0, 0, 200, 200);
    
    listView->contentSize = 0;
    listView->contentOffset = 0;
    
    listView->scroll = (PGB_ListViewScroll){
        .active = false,
        .start = 0,
        .end = 0,
        .time = 0,
        .duration = 0.15,
        .indicatorVisible = false,
        .indicatorOffset = 0,
        .indicatorHeight = 0
    };
    
    listView->selectedItem = -1;
    
    listView->direction = PGB_ListViewDirectionNone;
    
    listView->repeatLevel = 0;
    listView->repeatIncrementTime = 0;
    listView->repeatTime = 0;

    listView->crankChange = 0;
    listView->crankResetTime = 0;
    
    listView->model = (PGB_ListViewModel){
        .selectedItem = -1,
        .contentOffset = 0,
        .empty = true,
        .scrollIndicatorHeight = 0,
        .scrollIndicatorOffset = 0,
        .scrollIndicatorVisible = false
    };
    
    return listView;
}

void PGB_ListView_invalidateLayout(PGB_ListView *listView)
{
    
    int y = 0;
    
    for(int i = 0; i < listView->items->length; i++)
    {
        PGB_ListItem *item = listView->items->items[i];
        item->offsetY = y;
        y += item->height;
    }
    
    listView->contentSize = y;
    
    int scrollHeight = listView->frame.height - PGB_ListView_scrollInset * 2;
    
    bool indicatorVisible = false;
    if(listView->contentSize > listView->frame.height)
    {
        indicatorVisible = true;
    }
    listView->scroll.indicatorVisible = indicatorVisible;
    
    float indicatorHeight = 0;
    if(listView->contentSize > listView->frame.height && listView->frame.height != 0)
    {
        indicatorHeight = PGB_MAX(scrollHeight * (listView->frame.height / listView->contentSize), PGB_ListView_scrollIndicatorMinHeight);
    }
    listView->scroll.indicatorHeight = indicatorHeight;
}

void PGB_ListView_reload(PGB_ListView *listView){
    
    PGB_ListView_invalidateLayout(listView);
    
    int numberOfItems = listView->items->length;
    
    if(numberOfItems > 0)
    {
        if(listView->selectedItem < 0)
        {
            PGB_ListView_selectItem(listView, 0, false);
        }
        else if(listView->selectedItem >= numberOfItems)
        {
            PGB_ListView_selectItem(listView, numberOfItems - 1, false);
        }
    }
    else
    {
        listView->scroll.active = false;
        listView->contentOffset = 0;
        listView->selectedItem = -1;
    }
    
    listView->needsDisplay = true;
}

void PGB_ListView_update(PGB_ListView *listView){
    
    PDButtons pushed;
    PDButtons pressed;
    playdate->system->getButtonState(&pressed, &pushed, NULL);
    
    if(pushed & kButtonDown)
    {
        int nextIndex = listView->selectedItem + 1;
        if(nextIndex >= 0 && nextIndex < listView->items->length)
        {
            PGB_ListView_selectItem(listView, nextIndex, true);
        }
    }
    else if(pushed & kButtonUp)
    {
        int prevIndex = listView->selectedItem - 1;
        if(prevIndex >= 0 && prevIndex < listView->items->length)
        {
            PGB_ListView_selectItem(listView, prevIndex, true);
        }
    }
    
    listView->crankChange += PGB_App->crankChange;
    
    if(listView->crankChange != 0)
    {
        listView->crankResetTime += PGB_App->dt;
    }
    else
    {
        listView->crankResetTime = 0;
    }
    
    if(listView->crankChange > 0 && listView->crankChange >= PGB_ListView_crankMinChange)
    {
        int nextIndex = listView->selectedItem + 1;
        if(nextIndex >= 0 && nextIndex < listView->items->length)
        {
            PGB_ListView_selectItem(listView, nextIndex, true);
            listView->crankChange = 0;
        }
    }
    else if(listView->crankChange < 0 && listView->crankChange <= (-PGB_ListView_crankMinChange))
    {
        int prevIndex = listView->selectedItem - 1;
        if(prevIndex >= 0 && prevIndex < listView->items->length)
        {
            PGB_ListView_selectItem(listView, prevIndex, true);
            listView->crankChange = 0;
        }
    }
    
    if(listView->crankResetTime > PGB_ListView_crankResetMinTime)
    {
        listView->crankResetTime = 0;
        listView->crankChange = 0;
    }
    
    PGB_ListViewDirection old_direction = listView->direction;
    listView->direction = PGB_ListViewDirectionNone;
    
    if(pressed & kButtonUp)
    {
        listView->direction = PGB_ListViewDirectionUp;
    }
    else if(pressed & kButtonDown)
    {
        listView->direction = PGB_ListViewDirectionDown;
    }
    
    if(listView->direction == PGB_ListViewDirectionNone || listView->direction != old_direction)
    {
        listView->repeatIncrementTime = 0;
        listView->repeatLevel = 0;
        listView->repeatTime = 0;
    }
    else
    {
        listView->repeatIncrementTime += PGB_App->dt;
        
        float repeatInterval = PGB_ListView_repeatInterval1;
        if(listView->repeatLevel > 0)
        {
            repeatInterval = PGB_ListView_repeatInterval2;
        }
        
        if(listView->repeatIncrementTime >= repeatInterval)
        {
            listView->repeatLevel = PGB_MIN(3, listView->repeatLevel + 1);
            listView->repeatIncrementTime = fmodf(listView->repeatIncrementTime, repeatInterval);
        }
        
        if(listView->repeatLevel > 0)
        {
            listView->repeatTime += PGB_App->dt;
            
            float repeatRate = 0.16;
            
            if(listView->repeatLevel == 2)
            {
                repeatRate = 0.1;
            }
            else if(listView->repeatLevel == 3)
            {
                repeatRate = 0.05;
            }
            
            if(listView->repeatTime >= repeatRate)
            {
                listView->repeatTime = fmodf(listView->repeatTime, repeatRate);
                
                if(listView->direction == PGB_ListViewDirectionUp)
                {
                    int prevIndex = listView->selectedItem - 1;
                    if(prevIndex >= 0 && prevIndex < listView->items->length)
                    {
                        PGB_ListView_selectItem(listView, prevIndex, true);
                    }
                }
                else if(listView->direction == PGB_ListViewDirectionDown)
                {
                    int nextIndex = listView->selectedItem + 1;
                    if(nextIndex >= 0 && nextIndex < listView->items->length)
                    {
                        PGB_ListView_selectItem(listView, nextIndex, true);
                    }
                }
            }
        }
    }
    
    if(listView->scroll.active)
    {
        listView->scroll.time += PGB_App->dt;
        
        float progress = pgb_easeInOutQuad(fminf(1, listView->scroll.time / listView->scroll.duration));
        listView->contentOffset = listView->scroll.start + (listView->scroll.end - listView->scroll.start) * progress;
        
        if(listView->scroll.time >= listView->scroll.duration)
        {
            listView->scroll.time = 0;
            listView->scroll.active = false;
        }
    }
        
    float indicatorOffset = PGB_ListView_scrollInset;
    if(listView->contentSize > listView->frame.height)
    {
        int scrollHeight = listView->frame.height - (PGB_ListView_scrollInset * 2 + listView->scroll.indicatorHeight);
        indicatorOffset = PGB_ListView_scrollInset + (listView->contentOffset / (listView->contentSize - listView->frame.height)) * scrollHeight;
    }
    listView->scroll.indicatorOffset = indicatorOffset;
}

void PGB_ListView_draw(PGB_ListView *listView)
{
    bool needsDisplay = false;
    
    if(listView->model.empty || listView->needsDisplay || listView->model.selectedItem != listView->selectedItem || listView->model.contentOffset != listView->contentOffset || listView->model.scrollIndicatorVisible != listView->scroll.indicatorVisible || listView->model.scrollIndicatorOffset != listView->scroll.indicatorOffset || listView->scroll.indicatorHeight != listView->scroll.indicatorHeight)
    {
        needsDisplay = true;
    }
    
    listView->needsDisplay = false;
    
    listView->model.empty = false;
    listView->model.selectedItem = listView->selectedItem;
    listView->model.contentOffset = listView->contentOffset;
    listView->model.scrollIndicatorVisible = listView->scroll.indicatorVisible;
    listView->model.scrollIndicatorOffset = listView->scroll.indicatorOffset;
    listView->model.scrollIndicatorHeight = listView->scroll.indicatorHeight;

    if(needsDisplay)
    {
        int listX = listView->frame.x;
        int listY = listView->frame.y;

        playdate->graphics->fillRect(listX, listY, listView->frame.width, listView->frame.height, kColorWhite);
                
        for(int i = 0; i < listView->items->length; i++)
        {
            PGB_ListItem *item = listView->items->items[i];
            
            int rowY = listY + item->offsetY - listView->contentOffset;
            
            bool selected = (i == listView->selectedItem);
            
            if(selected)
            {
                playdate->graphics->fillRect(listX, rowY, listView->frame.width, item->height, kColorBlack);
            }
            
            if(item->type == PGB_ListViewItemTypeButton)
            {
                PGB_ListItemButton *itemButton = item->object;
                
                if(selected)
                {
                    playdate->graphics->setDrawMode(kDrawModeFillWhite);
                }
                else
                {
                    playdate->graphics->setDrawMode(kDrawModeFillBlack);
                }
                
                int textX = listX + PGB_ListView_inset;
                int textY = rowY + (float)(item->height - playdate->graphics->getFontHeight(PGB_App->subheadFont)) / 2;
                
                playdate->graphics->setFont(PGB_App->subheadFont);
                playdate->graphics->drawText(itemButton->title, strlen(itemButton->title), kUTF8Encoding, textX, textY);
                
                playdate->graphics->setDrawMode(kDrawModeCopy);
            }
        }
        
        if(listView->scroll.indicatorVisible)
        {
            int indicatorLineWidth = 1;

            PDRect indicatorFillRect = PDRectMake(listView->frame.width - PGB_ListView_scrollInset - PGB_ListView_scrollIndicatorWidth, listView->scroll.indicatorOffset, PGB_ListView_scrollIndicatorWidth, listView->scroll.indicatorHeight);
            PDRect indicatorBorderRect = PDRectMake(indicatorFillRect.x - indicatorLineWidth, indicatorFillRect.y - indicatorLineWidth, indicatorFillRect.width + indicatorLineWidth * 2, indicatorFillRect.height + indicatorLineWidth * 2);
            
            pgb_drawRoundRect(indicatorBorderRect, 2, indicatorLineWidth, kColorWhite);
            pgb_fillRoundRect(indicatorFillRect, 2, kColorBlack);
        }
    }
}

static void PGB_ListView_selectItem(PGB_ListView *listView, unsigned int index, bool animated){
    
    PGB_ListItem *item = listView->items->items[index];
    
    int listHeight = playdate->display->getHeight();
    
    int centeredOffset = 0;
    
    if(listView->contentSize > listHeight)
    {
        centeredOffset = item->offsetY - ((float)listHeight / 2 - (float)PGB_ListView_rowHeight / 2);
        centeredOffset = PGB_MAX(0, centeredOffset);
        centeredOffset = PGB_MIN(centeredOffset, listView->contentSize - listHeight);
    }
    
    if(animated)
    {
        listView->scroll.active = true;
        listView->scroll.start = listView->contentOffset;
        listView->scroll.end = centeredOffset;
        listView->scroll.time = 0;
    }
    else {
        listView->scroll.active = false;
        listView->contentOffset = centeredOffset;
    }
    
    listView->selectedItem = index;
}

void PGB_ListView_free(PGB_ListView *listView)
{
    
    array_free(listView->items);
    pgb_free(listView);
}

static PGB_ListItem* PGB_ListItem_new(void)
{
    PGB_ListItem *item = pgb_malloc(sizeof(PGB_ListItem));
    return item;
}

PGB_ListItemButton* PGB_ListItemButton_new(char *title)
{
    
    PGB_ListItem *item = PGB_ListItem_new();
    
    PGB_ListItemButton *buttonItem = pgb_malloc(sizeof(PGB_ListItemButton));
    buttonItem->item = item;
    
    item->type = PGB_ListViewItemTypeButton;
    item->object = buttonItem;
    
    item->height = PGB_ListView_rowHeight;
    
    buttonItem->title = string_copy(title);
    
    return buttonItem;
}

static void PGB_ListItem_super_free(PGB_ListItem *item)
{
    pgb_free(item);
}

void PGB_ListItemButton_free(PGB_ListItemButton *itemButton)
{
    PGB_ListItem_super_free(itemButton->item);
    
    pgb_free(itemButton->title);
    pgb_free(itemButton);
}

void PGB_ListItem_free(PGB_ListItem *item)
{
    if(item->type == PGB_ListViewItemTypeButton){
        PGB_ListItemButton_free(item->object);
    }
}
