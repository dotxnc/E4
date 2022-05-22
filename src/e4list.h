#ifndef E4LIST_H
#define E4LIST_H

#include "e4util.h"
#include <stdbool.h>

typedef struct ListT
{
    u32 len;
    u32 _max;
    void** data;
} ListT;

ListT* e4list_create();
void e4list_push(ListT* list, void* ptr);
void e4list_append(ListT* list, void* ptr);
void* e4list_get(ListT* list, u32 index);
void* e4list_remove(ListT* list, u32 index);
void* e4list_delete(ListT* list, void* ptr);
bool e4list_has(ListT* list, void* ptr);
void e4list_free(ListT* list);
void* e4list_empty(ListT* list);

#endif
