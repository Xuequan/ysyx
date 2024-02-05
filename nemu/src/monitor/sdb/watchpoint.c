/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#include <utils.h>

#define NR_WP 32

// I move this struct WP in sdb.h
/*
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  // TODO: Add more members if necessary 
	char *expr;
	word_t val; // expr value
} WP;
*/

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
		
		// initial wp.expr[]
		for (int m = 0; m < WP_EXPR_LENGTH; m++) {
			wp_pool[i].expr[m] = '\0';
		}
		wp_pool[i].val = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp();
void free_wp(int num);

/* add a wp to list(free_ or head) tail */
void add_wp2tail(WP** list, WP* wp) {
	if (wp == NULL) {
		printf("wp is NULL\n");
		return;
	}
	if (*list == NULL) {
		*list = wp;
	}else {
		WP *ptr = *list;
		for (; ptr->next != NULL; ptr = ptr->next) 
			{ ;}
		ptr->next = wp;
		wp->next = NULL;
	}
}

/* get a new wp from free_   */
WP* new_wp() {
	// free_ has no wp
	if (free_ == NULL) {
		printf("watchpoints fully used.\n");
		return NULL;
	} 
	// free_ has only 1 wp
	else if (free_->next == NULL) {
		WP* ptr = free_;
		add_wp2tail(&head, ptr);
		free_ = NULL;		
		return ptr; 
	} 
	else {    // free_ has more than 1 WP
		WP *ptr = free_;
		WP *next = free_->next;
		free_ = next;

		/* ? */
		ptr->next = NULL;

		add_wp2tail(&head, ptr);
		if (head == NULL)
			printf("here3\n");
		return ptr;
	}
}

/* clear a wp's content */
static void clear_wp(WP* wp) {
	if (wp == NULL) {
		assert(0);
	}
	// clear wp.expr and val
	for (int m = 0; m < WP_EXPR_LENGTH; m++) {
		wp->expr[m] = '\0';
	}
	wp->val = 0;
}

/* free a wp: 1. remove this wp from head;
** 2. clear its content
** 3. add this wp to free;
** input the NO of wp 
*/
void free_wp(int num) {
	if (head == NULL) {
		printf("Error: no existed watchpoint.");
		assert(0);
	}
	WP* ptr = head;
	for ( ; ptr != NULL && ptr->NO != num; ptr = ptr->next) {
		;
	}
	if (ptr == NULL) {
		printf("watchpoint %d is not exist\n", num);
		assert(0);
	} 

	// ptr->NO == num 
	clear_wp(ptr);
	if (ptr == head) {   // only 1 watchpoint (head ifself)
		add_wp2tail(&free_, ptr);
		head = NULL;
	} else {
		WP* prev = head;
		for ( ; prev != ptr; prev = prev->next) {	;}
		prev->next = ptr->next;
		ptr->next = NULL;
		add_wp2tail(&free_, ptr);
	}
}

/* scan watchpoint and see if the expr value change */
void scan_wp_pool() {
  WP* ptr = head;
  word_t now_result;
  bool success = false;
  for( ;ptr != NULL; ptr = ptr->next) {
    now_result = expr(ptr->expr, &success);
    if (success == false) {
      printf("trace_and_difftest(): expr() error\n");
      assert(0);
    }
    if (now_result != ptr->val) {
      nemu_state.state = NEMU_STOP;
      printf("Hardware watchpoint %d: val\n", ptr->NO);
      printf("\n");
      printf("Old vaule = %d\n", ptr->val);
      printf("New value = %u\n", now_result);
    }
  }// end for (;...)
}

/* for command "info w" */
void print_wp() {
	if (head == NULL) {
		printf("No watchpoints.\n");
		return;
	}
	printf("Num Type          Disp Enb Address    What  \n");
	WP* ptr = head;
	for ( ;ptr != NULL; ptr = ptr->next) {
		// Num Type Disp Enb Address What
		printf("%-3d %-10s %-4s %-3s %-10s %-6s\n", 
			ptr->NO, "hw watchpoint", "keep", "y", ptr->expr+1, "val");
	}	
} // end function
