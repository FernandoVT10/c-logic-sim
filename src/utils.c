#include <stdlib.h>
#include <assert.h>

#include "utils.h"

void *alloc(size_t bytes) {
    void *ptr = calloc(1, bytes);
    assert(ptr != NULL && "Error allocating memory");
    return ptr;
}

Set *set_new() {
    return alloc(sizeof(Set));
}

static SetItem *item_new(void *data) {
    SetItem *item = alloc(sizeof(SetItem));
    item->data = data;
    return item;
}

void set_add(Set *set, void *data) {
    SetItem *item = item_new(data);

    if(set->count == 0) {
        set->head = set->tail = item;
    } else {
        // we set the "next" of the last item to this new item
        set->tail->next = item;
        // then we set the last item to this new item
        set->tail = item;
    }

    set->count++;
}

bool set_delete(Set *set, void *data) {
    SetItem *item = set->head;
    SetItem *prevItem = NULL;

    size_t pos = 0;

    while(item != NULL) {
        // here we compare pointers to see if it's the item we're looking for
        if(item->data == data) {
            if(set->count == 1) {
                // if the set only has one element, we can only empty the list
                set->tail = set->head = NULL;
            } else if(pos == 0) {
                // if the position is 0 but there's more than 1 item
                set->head = item->next;
            } else if(pos == set->count - 1) {
                // if the position is the last item of the list and there's more than 1 item
                // remove the link between the previous item and this last one
                prevItem->next = NULL;
                // set the tail to the previous item
                set->tail = prevItem;
            } else {
                // if we're in the middle we only connect the previous to the next
                prevItem->next = item->next;
            }
            free(item);
            set->count--;
            return true;
        }
        prevItem = item;
        item = item->next;
        pos++;
    }

    return false;
}

void set_clear_and_destroy(Set *set) {
    SetItem *item = set->head;
    while(item != NULL) {
        SetItem *currentItem = item;
        item = item->next;
        free(currentItem);
    }

    free(set);
}
