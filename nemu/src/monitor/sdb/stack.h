/* stack interface
** 2023-9-29
*/


#ifndef stack_type 

#include <stddef.h>
#define STACK_TYPE int

/* create_stack
**		No need because of linked list structure stack.
*/
void create_stack(size_t size);

/* destroy_stack
**
*/
void destroy_stack(void);

/* push
**	Push a element into the stack.
**	Argument is the value. 
**	Return nothing.
*/
void push(STACK_TYPE value);

/* pop
**		Pop the top element off of the stack.
**		No argument. No return.
*/
void pop(void);

/* top
**		Return the topmost element of the stack.
**		Non't change the stack.
*/
STACK_TYPE top(void);

/* is_empty
** 		Return 1 if the stack is empty. 
*/
int is_empty(void);

/* is_full
**		No need because of linked list structure stack.
*/
int is_full(void);

void print_stack(void);
#endif
