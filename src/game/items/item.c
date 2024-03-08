#include "item.h"

#include <stdlib.h>

IItem* itemCreate() {
    return (IItem*) malloc(sizeof(IItem));
}

void itemDestroy(IItem* item) {
    free(item);
}