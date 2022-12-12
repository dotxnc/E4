#include "e4list.h"
#include <stdlib.h>
#include <string.h>

ListT* e4list_create()
{
    ListT* list = malloc(sizeof(ListT));
    list->data = malloc(sizeof(void*) * 8);
    list->len = 0;
    list->_max = 8;
    return list;
}

void e4list_push(ListT* list, void* ptr)
{
    if (list->len == list->_max)
        list->data = realloc(list->data, sizeof(void*) * list->_max * 2);

    memcpy(&list->data[1], list->data, sizeof(void*) * list->_max);
    list->data[0] = ptr;

    if (list->len == list->_max)
        list->_max *= 2;
    list->len++;
}

void e4list_append(ListT* list, void* ptr)
{
    if (list->len == list->_max)
        list->data = realloc(list->data, sizeof(void*) * list->_max * 2);

    list->data[list->len] = ptr;

    if (list->len == list->_max)
        list->_max *= 2;
    list->len++;
}

void* e4list_get(ListT* list, u32 index)
{
    return list->data[index];
}

void* e4list_remove(ListT* list, u32 index)
{
    void* ptr = list->data[index];
    list->data[index] = NULL;
    memmove(&list->data[index], &list->data[index + 1], sizeof(void*) * (list->_max - 1 - index));
    list->len--;
    return ptr;
}

void* e4list_delete(ListT* list, void* ptr)
{
    i32 index = -1;
    for (i32 i = 0; i < list->len; i++)
    {
        if (list->data[i] == ptr)
        {
            index = i;
            break;
        }
    }

    if (index > -1)
        return e4list_remove(list, index);
    return NULL;
}

bool e4list_has(ListT* list, void* ptr)
{
    for (i32 i = 0; i < list->len; i++)
        if (list->data[i] == ptr)
            return true;
    return false;
}

void e4list_free(ListT* list)
{
    for (i32 i = 0; i < list->len; i++)
        free(list->data[i]);
}

void* e4list_empty(ListT* list)
{
    free(list->data);
    free(list);
    return e4list_create();
}
