#include "item.h"

#include <stdlib.h>

const IItem IITEM_NULL = (IItem) {
    .self = NULL,
    .vptr = NULL
};

IItem* itemCreate() {
    return (IItem*) malloc(sizeof(IItem));
}

void itemDestroy(IItem* item) {
    free(item);
}