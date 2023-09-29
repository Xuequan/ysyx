/* stack implenmentation */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "stack.h"

typedef struct StackNode {
	STACK_TYPE value;
	struct StackNode *next;
} StackNode;

/* stack pointer to the first element */
static StackNode *stack;  

void create_stack(size_t size) {
}

void destroy_stack(void) {
	while(!is_empty()) {
		pop();
	}
	stack = NULL;
}

void push(STACK_TYPE value) {
	StackNode *first = malloc(sizeof(StackNode));

	assert(first != NULL);
	first->next = stack;
	first->value = value;
	stack = first;
}

void pop(void) {
	assert(stack != NULL);
	
	StackNode *top = NULL;
	top = stack;
	stack = top->next;
	free(top);
}

STACK_TYPE top(void) {
	assert(stack != NULL);
	
	return stack->value;
}

int is_empty(void) {
	return stack == NULL;
}

int is_full(void) { 
	return 0;
}

void print_stack(void) {
	while(!is_empty()) {
		printf("====\n");
		printf("%d", top());
		pop();
	}
	printf("\n");
	printf("==========\n");
}
		
		

