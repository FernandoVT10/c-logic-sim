#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#define DA_INIT_CAP 16

#define da_append(da, item)                                                          \
    do {                                                                             \
        if((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while(0)

#define da_free(da) do { free((da)->items); } while(0)

#define panic(message) do{fprintf(stderr, "ERROR: %s at %s:%d\n", message, __FILE__, __LINE__);exit(1);}while(0)

typedef struct SetItem SetItem;

struct SetItem {
    void *data;
    SetItem *next;
};


// the set has is a linked list with a "tail" to be able to add new items easily
// the set deletes or finds an element using the "data" field of the SetItem
// so, it works comparing pointers
typedef struct {
    SetItem *head;
    SetItem *tail;
    size_t count;
} Set;

Set *set_new();
void set_add(Set *set, void *data);
void set_delete(Set *set, void *data);
void set_clear_and_destroy(Set *set);

void *alloc(size_t bytes);

#endif // UTILS_H
