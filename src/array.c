//
//  array.c
//  PlayGB
//
//  Created by Matteo D'Ignazio on 23/08/21.
//

#include "array.h"

PGB_Array* array_new(void)
{
    PGB_Array *array = pgb_malloc(sizeof(PGB_Array));
    array->length = 0;
    array->items = NULL;

    return array;
}

void array_push(PGB_Array *array, void *item)
{
    array->length++;
    array->items = pgb_realloc(array->items, array->length * sizeof(item));
    array->items[array->length - 1] = item;
}

void array_clear(PGB_Array *array)
{
    array->length = 0;
    pgb_free(array->items);
    array->items = NULL;
}

void array_free(PGB_Array *array)
{
    pgb_free(array->items);
    pgb_free(array);
}
