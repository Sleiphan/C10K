#include <stdlib.h>

#include "unix/stack.h"


stack stack_create() {
    stack s;
    s.top = (stack_node*) 0;
    return s;
}

void stack_push(stack* s, void* val) {
    stack_node* new_node = (stack_node*) malloc(sizeof(stack_node));
    new_node->data = val;
    new_node->prev = s->top;

    s->top = new_node;
}

void* stack_pop(stack* s) {
    void* val = s->top->data;
    stack_node* new_top = s->top->prev;

    free(s->top);
    s->top = new_top;

    return val;
}

inline int stack_is_empty(stack s) {
    return s.top == (void*) 0;
}

void stack_empty(stack* s) {
    while (s->top != 0) {
        stack_node* new_top = s->top->prev;
        free(s->top);
        s->top = new_top;
    }
}

inline void stack_destroy(stack* s) {
    stack_empty(s);
}