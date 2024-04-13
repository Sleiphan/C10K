#include <stdlib.h>
#include <string.h>

#include "unix/boolset.h"



boolset_t boolset_create() {
    boolset_t b;

    b.size = 1;
    b.data = (BOOLSET_TYPE*) malloc(b.size * sizeof(BOOLSET_TYPE));

    memset(b.data, 0, b.size * sizeof(BOOLSET_TYPE));

    return b;
}



void boolset_expand(boolset_t* set, BOOLSET_TYPE new_size) {
    const BOOLSET_TYPE old_byte_size = set->size * sizeof(BOOLSET_TYPE);
    const BOOLSET_TYPE new_byte_size = new_size * sizeof(BOOLSET_TYPE);

    BOOLSET_TYPE* old_data = set->data;
    BOOLSET_TYPE* new_data = malloc(new_byte_size); // Allocate new memory

    memset(new_data, 0, new_byte_size); // Initialize data to 0
    memcpy(new_data, old_data, old_byte_size); // Copy old data to new memory location
    free(old_data);

    // Store the new data and size
    set->data = new_data;
    set->size = new_size;
}



void boolset_set(boolset_t* set, BOOLSET_TYPE index, int val) {
    const BOOLSET_TYPE major_index = index / (sizeof(BOOLSET_TYPE) * 8);

    const BOOLSET_TYPE required_size = major_index + 1;

    if (required_size > set->size)
        boolset_expand(set, required_size);

    const BOOLSET_TYPE minor_index = index % (sizeof(BOOLSET_TYPE) * 8);

    // Normalize the value of 'val'
    val = val != 0;

    // A mask to the requested bit
    const BOOLSET_TYPE mask = (BOOLSET_TYPE) 1 << minor_index;

    BOOLSET_TYPE temp = set->data[major_index] & ~mask;
    temp |= mask * val;

    set->data[major_index] &= ~mask; // Set the desired bit to 0
    set->data[major_index] |= mask * val; // OR in the desired bit to the submitted value
}



int boolset_get(const boolset_t* set, BOOLSET_TYPE index) {
    const BOOLSET_TYPE major_index = index / (sizeof(BOOLSET_TYPE) * 8);
    const BOOLSET_TYPE minor_index = index % (sizeof(BOOLSET_TYPE) * 8);

    const BOOLSET_TYPE mask = (BOOLSET_TYPE) 1 << minor_index;

    return (set->data[major_index] & mask) != ((BOOLSET_TYPE) 0);
}



void boolset_destroy(boolset_t* b) {
    free(b->data);
}


void boolset_foreach(const boolset_t* set, void(*callback)(BOOLSET_TYPE index, void* args), void* args) {
    for (BOOLSET_TYPE major_index = 0; major_index < set->size; ++major_index) {
        const BOOLSET_TYPE value = set->data[major_index];

        if (value == 0)
            continue;

        for (int minor_index = 0; minor_index < sizeof(BOOLSET_TYPE) * 8; ++minor_index) {
            if ((value & ((BOOLSET_TYPE) 1 << minor_index)) == 0)
                continue;
            
            callback(major_index * sizeof(BOOLSET_TYPE) * 8 + minor_index, args);
        }
    }
}