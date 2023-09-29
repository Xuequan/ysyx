/* stack implenmentation */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stddef.h>
//#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.h"
#include "stack.h"

typedef struct StackNode {
	STACK_TYPE value;
	struct StackNode *next;
} StackNode;

// stack pointer to the first element 
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
	assert(!is_empty());
	
	StackNode *top = NULL;
	top = stack;
	stack = top->next;
	free(top);
}

STACK_TYPE top(void) {
	assert(!is_empty());
	
	return stack->value;
}

int is_empty(void) {
	return stack == NULL;
}

int is_full(void) { 
	return 0;
}

void print_stack(void) {
	printf("====start====\n");
	for( StackNode *ptr = stack; ptr != NULL; ptr = ptr->next) {
		printf(" %d", ptr->value);
	}
	printf("\n");
	printf("====end====\n");
}

