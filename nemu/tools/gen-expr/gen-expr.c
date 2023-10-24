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

#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.h"
#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.c"

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { \n"
"  unsigned result = %s; \n"
//"  unsigned long long result = (unsigned long long)%s; \n"
//"  printf(\"%%llu\", result); \n"
"  printf(\"%%u\", result); \n"
"  return 0; \n"
"} \n";

static int div_by_zero = 0;
static int buf_overflow = 0;

static int op_is_div = 0;

uint32_t choose(uint32_t n);
int buf_length(void); 
void gen_num(void);
void gen(char c);
bool gen_rand_op(void);
bool buf_is_overflow(void);
static void gen_rand_expr(int *deep);

int find_expr(int j, int length);
uint32_t eval_expr(int p, int q);
void char_array_initial(char array[], int start, int length);
uint32_t eval(char expr[], int length);


uint32_t choose(uint32_t n) {
	return (uint32_t) (rand() % n);	
}

/* calculate the length of buf[65536];
** return -1 if overflow buf
*/
int buf_length(void) {
	uint32_t i = 0;
	for(; buf[i] != '\0'; i++) 
		;
	// buf[65536] overflow
	if (i > 65535) {
		return -1;
	}
	return i;
}

void gen_num(void) {
	if (buf_is_overflow() ) {
		return ;
	}
	int i = 0;
	//time_t t;
	//int num = 0;
	//srand((unsigned) time(&t));

	uint32_t num = (uint32_t)(rand() % 100);
	//uint32_t num = (uint32_t)rand();

	// add a num to the tail of buf[65535]
	/*
	char *ptr = (char *)&num;
	uint32_t len = buf_length();
	memcpy(buf + len, ptr, sizeof(uint32_t));
	*/
	
	char temp[11];
	int len = buf_length();	
	snprintf(temp, sizeof(temp), "%u", num);
	for (; temp[i] != '\0'; i++) {
		buf[len + i] = temp[i];
	}
	buf[len + i] = ' ';
	buf[len + i + 1] = '\0';
	//buf[len + i] = '\0';
}

void gen(char c) {

	if (buf_is_overflow() ) {
		return;
	}

	int len = buf_length();
	buf[len] = c;
	buf[len + 1] = ' ';
	buf[len + 2] = '\0';
	//buf[len + 1] = '\0';
}

/*	return true if buf overflow */	
bool buf_is_overflow(void) {
	if (buf_length() == -1 ) {
		buf_overflow = 1;
		return true;
	}
	return false;
}

/* if generated op is div, then return true */
bool gen_rand_op(void) {

	if (buf_is_overflow() ) {
		return false;
	}
	char op[] = "+-*/";
	//char op[] = "+-*";
	uint32_t index = choose(4);
	gen(op[index]);
	if (index == 3) {
		return true;
	} else {
		return false;
	}
}

/* control the length of expr */
#define DEPTH 10

static void gen_rand_expr(int *deep) {
	/*
	time_t t;
	srand((unsigned) time(&t));
  buf[0] = '\0';
	*/
	(*deep)++;

	if (buf_is_overflow() ) {
		return;
	}
	if (*deep > DEPTH) {
		gen_num();
		return;
	}
	//printf("*deep = %d\n", *deep);

	switch(choose(3)) {
		case 0: gen_num(); break;
		case 1: gen('('); 
						gen_rand_expr(deep); 
						gen(')'); 
						break;
		default:  
						gen_rand_expr(deep); 
						gen_rand_op();
						gen_rand_expr(deep); 
						break;
	}// end switch 
}

/* return true if division by zero */
/* 前提条件是，输入的表达式，无论是 op 还是 num 
** 都是由空格隔开的
*/
bool check_div_by_zero(void) {
	//printf("buf = %s\n", buf);
	int length = buf_length();
	int i = 0;
	int j = 0;
	int begin = 0;
	int end = 0;
	int k = 0;
	char num[length];
	char_array_initial(num, 0, length);

	char expr[length];
	char_array_initial(expr, 0, length);
	// travel the whole buf[]
	for(; i < length; i++) {
		if(buf[i] == '/') {
			j = i + 1;
			// skip whitespace ' '
			while(buf[j] == ' ') { 
				j++;
			}

			switch(buf[j]) {
				case '(':
					// 若是括号，则找到第一个完整的式子
					//printf("here: buf[%d] = %c\n", j, buf[j]);
					k = find_expr(j, length);
					if (eval_expr(j, k) == 0) {
						return true;
					}
					break;

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					// 若是数字，则找出这个完整的数字
					// 判断它是否为0即可
					
					// 找到除号后的第一个数字的开始
					begin = j;	
					while(buf[j++] != ' ') { }
					// 找到除号后的第一个数字后的空格
					end = j;
					memcpy(num, buf + begin, end - begin);
					num[end - begin] = '\0';
					//printf("here: num  = %s, (int)num = %d, atoi(num) = %d\n", 
					//		num, *(int *)num, (uint32_t)atoi(num));
					if ((uint32_t)atoi(num) == 0) {
						return true;
					}
					break;
				default: 
					printf("check_div_by_zero(): expr error. buf[%d] = %c\n", j, buf[j]);
					assert(0);
			} // end switch
		} // end if
	} // end for(; i ...)
	return false;
}

uint32_t eval_expr(int p, int q) {
	uint32_t result = 0;

	char expr[q - p];
	char_array_initial(expr, 0, q-p);

	// copy from buf[p+1] to buf[q-1]
	// skip '(' and ')'
	memcpy(expr, buf + p + 1, q - p - 1);
	//expr[q - p + 1] = '\0';

	result = eval(expr, q - p - 2);
	//uint32_t result = (uint32_t)atoi(expr);	
	//printf("eval(%d, %d), expr = %s, result = %d\n", p , q, expr, result);
	return result;
}

uint32_t eval(char expr[], int length) {
	char eval_code[length + 128];
	char_array_initial(eval_code, 0, length + 128);
	
	sprintf(eval_code, code_format, expr);
	FILE *fp2 = fopen("/home/chuan/Templates/.expr.c", "w");
	assert(fp2 != NULL);
	fputs(eval_code, fp2);
	fclose(fp2);
	
	int ret2 = system("gcc /home/chuan/Templates/.expr.c -o /home/chuan/Templates/.expr");
	if (ret2 != 0) {
		printf("system() error!\n");
		assert(0);
	}
	
	fp2 = popen("/home/chuan/Templates/.expr", "r");
	assert(fp2 != NULL);
	int result = 0;
	ret2 = fscanf(fp2, "%d", &result);
	pclose(fp2);
	return (uint32_t)result;
}

void char_array_initial(char array[], int start, int length) {
	for(int i = start; i < length; i++) {
		array[i] = '\0';
	} 
}
int find_expr(int j, int length) {
	//printf("find_expr(%d, %d)\n", j, length);
	int i = j;
	for( ; i < length; i++) {
		switch(buf[i]) {
			case '(': 
				push('(');
				//print_stack("find_expr");
				break;
			case ')':
				if (is_empty() ) {
					printf("find_expr(): error1\n");
					assert(0);
				}
				pop();
				if (is_empty() ) {
					destroy_stack();
					return i;	
				}
				break;
			default: break;	
		} // end switch
	}// end for
	if (i == length) {
		printf("find_expr(): error\n");
		return 0;
	}
}

int main(int argc, char *argv[]) {

	//div_by_zero = 0;  // global var
	//buf_overflow = 0;

	int *deep = (int *)malloc(sizeof(int *));

  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {

		/* chuan */
		buf[0] = '\0';
		*deep = 0;
		buf_overflow = 0;

    gen_rand_expr(deep);
		
		if (buf_is_overflow() || 
				check_div_by_zero()) {
			continue;
		}

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
  }
	free(deep);
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

