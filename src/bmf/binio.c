#include "bmf_internal.h"

#include <stdlib.h>
#include <string.h>

#define CAPACITY_INCREMENT 1024

bmf_binary* bin_create() {
    return bin_alloc(CAPACITY_INCREMENT);
}

bmf_binary* bin_alloc(int size) {
    bmf_binary* bin = malloc(sizeof(bmf_binary));
    bin->capacity = (size + (CAPACITY_INCREMENT - 1)) / CAPACITY_INCREMENT; // effective ceil
    bin->ptr = 0;
    bin->data = calloc(bin->capacity, 1);
    return bin;
}

bmf_binary* bin_wrap(char* data, int length) {
    bmf_binary* bin = bin_alloc(length);
    bin_write(bin, data, length);
    return bin;
}

void bin_read(bmf_binary* bin, void* dst, int size) {
    memcpy(dst, bin->data + bin->ptr, size);
    bin->ptr += size;
}

void bin_write(bmf_binary* bin, void* src, int size) {
    int prev_capacity = bin->capacity;
    while (bin->ptr + size >= bin->capacity) {
        bin->capacity += CAPACITY_INCREMENT;
    }
    if (prev_capacity != bin->capacity) bin->data = realloc(bin->data, bin->capacity);
    memcpy(bin->data + bin->ptr, src, size);
    bin->ptr += size;
}