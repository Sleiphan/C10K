#ifndef CUSTOM_UNIX_QUEUE
#define CUSTOM_UNIX_QUEUE

typedef struct queue {
    long _front;
    long _back;
    long _size;
    long _range_mask;

    void **_data;
} queue;

queue q_create();

void q_push(queue*, void* val);

int q_is_empty(queue);

void* q_next(queue*);

void q_resize(queue *q, long new_size);

void q_destroy(queue);




#endif // CUSTOM_UNIX_QUEUE