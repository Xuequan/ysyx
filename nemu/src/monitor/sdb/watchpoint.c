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

#define NR_WP 32
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

		wp_pool[i].expr = NULL;
		wp_pool[i].val = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp();
void free_wp(WP *wp);


WP* new_wp() {
	if (free_->next == NULL) {
		printf("watchpoint is full used.\n");
		return NULL;
	} 
	// get a wp from free_ tail
	WP *prev = free_;
	WP *ptr = prev->next;
	for ( ; ptr->next != NULL; ptr = ptr->next, prev = prev->next) {
		;
	}
	prev->next = NULL;
	// add this wp to head
	WP *ptr2 = head;
	for (; ptr2->next != NULL; ptr2 = ptr2->next) {
		;
	}
	ptr2->next = ptr;
	return ptr;	
}

void free_wp(WP *wp) {
	// delete wp from head 
	WP *prev = head;
	WP *ptr  = prev->next;
	for ( ;ptr != wp; prev = prev->next, ptr = ptr->next) { ; }

	if (ptr == NULL) {
		printf("wp is not exist, cannot free!\n");
		assert(0);
	}
	prev->next = ptr->next;	
	// add wp to free_ tail
	ptr = free_;
	for ( ;ptr->next != NULL; ptr = ptr->next) { ;}
	ptr->next = wp;
}
