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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

//#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.h"

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { \n"
"  unsigned result = %s; \n"
"  printf(\"%%u\", result); \n"
"  return 0; \n"
"} \n";

static int div_by_zero = 0;
uint32_t choose(uint32_t n);
uint32_t buf_length(void); 
void gen_num(void);
void gen(char c);
bool gen_rand_op(void);
/*
bool check_div_by_zero(uint32_t p, uint32_t q);
uint32_t eval_expr(uint32_t p, uint32_t q);
int find_main_op2(uint32_t p, uint32_t q, int *expr_val);
static uint32_t gen_rand_expr();
static bool check_parentheses(int p, int q, int option);
*/


uint32_t choose(uint32_t n) {
	return (uint32_t) (rand() % n);	
}

// calculate the length of buf[65536];
uint32_t buf_length(void) {
	uint32_t i = 0;
	for(; buf[i] != '\0'; i++) 
		;
	return i;
}
void gen_num(void) {
	int i = 0;
	//time_t t;
	//int num = 0;
	//srand((unsigned) time(&t));
	uint32_t num = (uint32_t)(rand() % 10);

	// add a num to the tail of buf[65535]
	/*
	char *ptr = (char *)&num;
	uint32_t len = buf_length();
	memcpy(buf + len, ptr, sizeof(uint32_t));
	*/
	

	char temp[11];
	uint32_t len = buf_length();	
	snprintf(temp, sizeof(temp), "%d", num);
	for (; temp[i] != '\0'; i++) {
		buf[len + i] = temp[i];
	}
	buf[len + i] = ' ';
	buf[len + i + 1] = '\0';
	//buf[len + i] = '\0';
}

void gen(char c) {
	uint32_t len = buf_length();
	buf[len] = c;
	buf[len + 1] = ' ';
	buf[len + 2] = '\0';
	//buf[len + 1] = '\0';
}

/* if generated op is div, then return true */
bool gen_rand_op(void) {
	//char op[] = "+-*/";
	char op[] = "+-*";
	uint32_t index = choose(3);
	gen(op[index]);
	if (index == 3) {
		return true;
	} else {
		return false;
	}
}

/* return the added length of buf[] 
** eg: beginning buf[] = {1, 2, 3}
** after gen_rand_expr() buf[] = {1,2,3,4,5}
** then return 2
*/
static uint32_t gen_rand_expr() {
	/*
	time_t t;
	srand((unsigned) time(&t));
  buf[0] = '\0';
	*/
	//printf("buf = %s\n", buf);

	uint32_t len1 = buf_length();
	bool gen_op_is_div = false;
	uint32_t len2 = 0;

	switch(choose(3)) {
		case 0: gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); 
						 gen_rand_op();
						 gen_rand_expr();
						 break;
		/*
		default: 
			gen_rand_expr();
			gen_op_is_div = gen_rand_op();
			printf("2-buf = %s, length = %d\n", buf, buf_length());
	
			len2 = gen_rand_expr();
			printf("3-buf = %s, length = %d, len2= %d\n", buf, buf_length(), len2);

			if (gen_op_is_div) {
				if ( check_div_by_zero(buf_length() - len2, buf_length() - 1) ) {
					printf("div by zero \n");
					div_by_zero = 1;  // global variable
				}
			}

			break;
		*/

	}// end switch 

	return buf_length() - len1;
}

int main(int argc, char *argv[]) {

	div_by_zero = 0;  // global var

  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

		if (div_by_zero == 1) {
			continue;
		}

	//	printf("buf = %s\n", buf);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
		/* chuan */
		buf[0] = '\0';
  }
  return 0;
}

/* if expr (buf[p, q]) == 0, return true */ 
/*
bool check_div_by_zero(uint32_t p, uint32_t q) {
	printf("check_div_by_zero(%d, %d), length = %d\n", p, q, buf_length() );
	return eval_expr(p, q) == 0;
}
*/

/* eval_expr(p, q) refer to eval(p, q) which is locate
** in nemu/src/monitor/sdb/expr.c.
*/
/*
uint32_t eval_expr(uint32_t p, uint32_t q) {
	int op = 0;
	uint32_t val1 = 0;
	uint32_t val2 = 0;
	
	uint32_t *expr_val = NULL;
	
	printf("eval_expr(%d, %d), length = %d\n", p, q, buf_length() );

	if (p > q) {
		printf("eval_expr(): bad expression\n");
		assert(0);
	} else if (p == q) {
		printf("eval_expr(%d, %d) return %d\n", p, q, atoi(buf+p));
		return (uint32_t)(atoi(buf+p));
	} else {
		op = find_main_op2(p, q, expr_val);
		printf("find_main_op2(%d, %d), op = %d\n", p, q, op);

		if ( op == -1) 
			return *expr_val;

		val1 = eval_expr(p, op - 1);
		val2 = eval_expr(op + 1, q);
		
		switch (buf[op]) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: 
				assert(0);
		} // end switch
	} // end if-else-if

}
*/
/*
int find_main_op2(uint32_t p, uint32_t q, int *expr_val) {
  int i = 0;
  int cnt = 0;
  int index[q - p];

  // find the operators between buf[p, q]
  for(i = p; i <= q; i++) {
		if (buf[i] == '+' || buf[i] == '-' ||
				buf[i] == '*' || buf[i] == '/') {
				index[cnt] = i;
				cnt++;
		} 
  }//end for


  // figure out which is the main operator
	if (cnt == 0) { // buf[p-q] has no op, so is a num
		for(int k = p; k <= q; k++) {
			if (buf[k] != '(' && buf[k] != ')') {
				*expr_val = atoi(buf + k);
				return -1;
			}
		}
	}
		
	expr_val = NULL;	

  int mul_div_index = 0;
  int plus_sub_index = 0;
  if (cnt == 1) {
    return index[0];
  } else {
    for ( i = 0; i < cnt; i++) {
			if (buf[index[i]] == '+' || buf[index[i]] == '-') {
        plus_sub_index = index[i];
			} else {
        mul_div_index = index[i];
      }
				
    }//end for
  }// end if-else

  if (plus_sub_index != 0) {
    return plus_sub_index;
  } else {
    return mul_div_index;
  }

} // end function
*/	
/* 
** check if the parentheses in the expr is legal 
** p and q is the index of tokens[]
**
** if argument option = 1, then 
**    the expr should be surrounded by a matched parentheses
** else argument option = 0, then
**    the expr no need surrounded by a matched parentheses 
*/
/*
static bool check_parentheses(int p, int q, int option) {
  printf("check_parentheses(%d,%d,%d)\n", p, q, option);
  int ii = p;
  int jj = q;

  if (p == q) {
    //printf("This is a number.\n");
    return true;
  }

  if (option == 1) {
		if (buf[p] != '(' || buf[q] != ')') {
    //if (tokens[p].type != TK_OPAREN || tokens[q].type != TK_CPAREN) {
      //printf("Leftmost '(' and rightmost ')' are not matched.\n");
      return false;
    }
  }

  for(; p <= q; p++) {
		switch(buf[p]) {
			case '(': push('('); break;
			case ')': 
				if (is_empty() || top() != '(') {
          printf("check_paren(%d, %d): bad expression\n", ii, jj);
          destroy_stack();
          return false;
        }
				pop();
				break;
			default:;
		} // end switch
	} // end for


	if ( !is_empty() ) {
		print_stack("not empty");
		destroy_stack();
		printf("check_paren2(%d, %d): bad expression\n", ii, jj);
		return false;
	}
  destroy_stack();
  return true;
}//end function
*/

