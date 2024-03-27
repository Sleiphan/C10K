#include <stdlib.h>
#include <limits.h>
#include "unix/queue.h"


const long CUSTOM_QUEUE_DEFAULT_SIZE = 8;



int MSB(long number) {
    int MSB = 63;
    unsigned long long val = (unsigned long long)1 << MSB;

    while (!(number & val) && MSB) {
        val >>= 1;
        MSB--;
    }
    
    return MSB;
}



queue q_create() {
    queue q;

    q._front = 0;
    q._back = 0;
    q._size = 0;

    q._data = (void**) malloc(sizeof(void*) * CUSTOM_QUEUE_DEFAULT_SIZE);
    q._range_mask = CUSTOM_QUEUE_DEFAULT_SIZE - 1;

    return q;
}

void q_resize(queue *q, long new_size) {
    int msb = MSB(new_size);

    if (new_size > (1 << msb))
        msb++;

    new_size = 1 << msb;

    // Skip of the sizes are the same
    if (new_size == q->_range_mask + 1)
        return;
    
    
    
    // Allocate new array
    void** new_data = (void**) malloc(new_size * sizeof(void*));

    // The number of elements to be moved to the new array
    const long move_size = q->_size < new_size ? q->_size : new_size;

    // Move data from the old array to the new array
    for (int i = 0; i < move_size; i++)
        new_data[i] = q_next(q);

    // Deallocate the old array
    free(q->_data);

    // Set the new array as the current array
    q->_data = new_data;

    // Update values
    q->_size = move_size;
    q->_range_mask = new_size - 1;
    q->_front = 0;
    q->_back = move_size;
    q->_back &= q->_range_mask;
}

void q_push(queue *q, void* val) {
    if (q->_size >= q->_range_mask + 1)
        q_resize(q, (q->_range_mask + 1) * 2);

    q->_data[q->_back] = val;
    
    q->_size++;
    q->_back++;
    q->_back &= q->_range_mask;
}

int q_is_empty(queue q) {
    return q._size == 0;
}

void* q_next(queue *q) {
    if (q->_size <= 0)
        return 0;

    void* next = q->_data[q->_front];

    q->_front++;
    q->_front &= q->_range_mask;
    q->_size--;

    return next;
}

void q_destroy(queue q) {
    free(q._data);
}