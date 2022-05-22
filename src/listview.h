//
//  listview.h
//  PlayGB
//
//  Created by Matteo D'Ignazio on 16/05/22.
//

#ifndef listview_h
#define listview_h

#include <stdio.h>
#include "utility.h"
#include "array.h"

typedef struct {
    bool empty;
    int contentOffset;
    int selectedItem;
} PGB_ListViewModel;

typedef enum {
    PGB_ListViewItemTypeButton,
    PGB_ListViewItemTypeSwitch
} PGB_ListItemType;

typedef struct {
    PGB_ListItemType type;
    void *object;
    int height;
    int offsetY;
} PGB_ListItem;

typedef struct {
    PGB_ListItem *item;
    char *title;
} PGB_ListItemButton;

typedef struct {
    PGB_Array *items;
    PGB_ListViewModel model;
    int selectedItem;
    int contentOffset;
    int contentSize;
    bool needsDisplay;
    PDRect frame;
} PGB_ListView;

PGB_ListView* PGB_ListView_new(void);

void PGB_ListView_update(PGB_ListView *listView);
void PGB_ListView_draw(PGB_ListView *listView);

void PGB_ListView_reload(PGB_ListView *listView);

void PGB_ListView_free(PGB_ListView *listView);

PGB_ListItemButton* PGB_ListItemButton_new(char *title);

void PGB_ListItem_free(PGB_ListItem *item);

#endif /* listview_h */
