#ifndef CUSTOM_STACK_IMPLEMENTATION
#define CUSTOM_STACK_IMPLEMENTATION



typedef struct stack_node {
    void* prev;
    void* data;
} stack_node;

typedef struct stack {
    stack_node* top;
} stack;

stack stack_create();

void stack_push(stack*, void* val);

void* stack_pop(stack*);

int stack_is_empty(stack);

void stack_empty(stack* s);

void stack_destroy(stack*);



#endif // CUSTOM_STACK_IMPLEMENTATION