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

#include <isa.h>
#include "stack.h"
//#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.h"
//#include "/home/chuan/ysyx-workbench/nemu/src/monitor/sdb/stack.c"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

void print_tokens(int nr_token);
word_t eval (int p, int q); 
int find_main_op(int p, int q);
static bool check_parentheses(int p, int q, int option);

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_VAL,   // decimal number
	TK_PLUS,  // plus
	TK_MINUS,	// minus 260
	TK_MUL,		// mul
	TK_DIV,		// div
	TK_OPAREN, // open parenthesis 263
	TK_CPAREN, // close parenthesis
	TK_NEWLINE, 
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    			// spaces
  {"\\+", TK_PLUS},     			// plus
  {"==", TK_EQ},        			// equal

	/* chuan start */
	{"[0-9]+", TK_VAL},  				// decimal numbers
	{"\\-", TK_MINUS},          // minus
	{"\\*", TK_MUL},					  // mul
	{"\\/", TK_DIV},					  // div
	{"\\(", TK_OPAREN},					// open parenthesis	
	{"\\)", TK_CPAREN},					// close parenthesis
	{"\\\n", TK_NEWLINE},        // newline
	/* end */
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
	
	//printf("make_token( %s )\n", e);

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

				/*
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
				*/
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

				/* copy the new token to a buffer token_str */
				if (substr_len > 32) {
					printf("token is too long.\n");
					assert(0);
				}

				// chuan: i donot know why below function cannot copy
				// maybe tokens[].str is a char array, not a char *
				//strncpy(tokens[nr_token].str, substr_start, (size_t) substr_len);
				// copy token to tokens[].str
				for (int j = 0; j < substr_len; j++)
					tokens[nr_token].str[j] = *(substr_start + j);
				tokens[nr_token].str[substr_len] = '\0';	
			
				/* print tokens[].str 	
				char * tmp = tokens[nr_token].str;
				printf("%d: tokens[%d].str = %s\n", nr_token, nr_token, tmp);
				*/

        switch (rules[i].token_type) {
					case TK_NOTYPE:    // if spaces, do not record
					case TK_NEWLINE:
						nr_token--; 
						break;				
					case TK_PLUS:  
					case TK_EQ: 	
					case TK_VAL: 	
					case TK_MINUS: 
					case TK_MUL: 
					case TK_DIV: 
					case TK_OPAREN: 
					case TK_CPAREN: 
						tokens[nr_token].type = rules[i].token_type;  
						break;
				
          default: printf("make_token(): unknown token_type \"%d\"\n", 
																				rules[i].token_type);
									 assert(0);
        }//end switch
				nr_token++;
        break;
      } 
    } // end for

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }//end while

	nr_token -= 1;

	//print_tokens(nr_token + 1);
		
  return true;
}

void print_tokens(int length) {
	for(int i = 0; i < length; i++) {
		printf("%d: tokens[%d].str = %s\n", i, i, tokens[i].str);
	}
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token);
}

// chuan, p < q
// only the final result will be word_t
// intermediate result is int 
word_t eval (int p, int q) {
	//printf("== eval(%d, %d)\n", p, q);
	int op = 0;
	int val1 = 0;
	int val2 = 0;

	if (p > q) {
		printf("eval(): bad expression\n");
		assert(0);
	} else if (p == q) {
		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		return (word_t)atoi(tokens[p].str);
	} else if (check_parentheses(p, q, 1) == true) {
		/* The expression is surrounded by a matched pair parentheses.
		 * If that is the case. just throw away the parentheses exper.
		 */
		return eval(p + 1, q - 1);
	} else if (check_parentheses(p, q, 0) == true) {
		op = find_main_op(p, q);
		//printf("============op = %d\n", op);
		val1 = (int)eval(p, op - 1);
		val2 = (int)eval(op + 1, q);
		
		switch (tokens[op].type) {
			case TK_PLUS: return val1 + val2;
			case TK_MINUS: return val1 - val2;
			case TK_MUL: return val1 * val2;
			case TK_DIV: 
				if (val2 == 0) {
					printf("div by zero error\n");
					assert(0);
				}
				return (word_t) (val1 / val2);
			default: assert(0);
		}//end switch
	} else if (check_parentheses(p, q, 0) == false) {
		printf("parentheses are not matched. Plese input again\n");
		assert(0);
	} else {
		printf("eval(): unknown error!\n");
		assert(0);
	}
}

/*
** find the position of 主运算符
*/
int find_main_op(int p, int q) {
	//printf("find_main_op(%d, %d)\n", p, q);
	int i = 0;
	int cnt = 0;
	int index[q - p];
	
	// find the operators between [p, q]
	for(i = p; i <= q; i++) {
		if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS
		 || tokens[i].type == TK_MUL  || tokens[i].type == TK_DIV) {
			if( (check_parentheses(p, i-1, 0) == true) && 
					(check_parentheses(i + 1, q, 0) == true) ) {
					index[cnt] = i;
					cnt++;
			}
		}
	}//end for

	// figure out which is the main operator
	int mul_div_index = 0;
	int plus_sub_index = 0;
	if (cnt == 1) {
		return index[0];
	} else {
		for ( i = 0; i < cnt; i++) {
			if (tokens[index[i]].type == TK_PLUS || 
					tokens[index[i]].type == TK_MINUS) {
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
}//end function


/* 
** check if the parentheses in the expr is legal 
** p and q is the index of tokens[]
**
** if argument option = 1, then 
** 		the expr should be surrounded by a matched parentheses
** else argument option = 0, then
**		the expr no need surrounded by a matched parentheses 
*/
static bool check_parentheses(int p, int q, int option) { 
	//printf("check_parentheses(%d,%d,%d)\n", p, q, option);
	int ii = p;
	int jj = q;
	if (p == q) {
		//printf("This is a number.\n");
		return true;
	}
	// 这里有问题
	// (76)/(67) 就不是被（）包围，但是这里显示的是 true
	// 故需要做第二次检查
	if (option == 1) {
		if (tokens[p].type != TK_OPAREN || tokens[q].type != TK_CPAREN) { 
			//printf("Leftmost '(' and rightmost ')' are not matched.\n");
			return false;
		}
	}

	for(; p <= q; p++) {
		switch(tokens[p].type) {
			case TK_OPAREN: 
				push(TK_OPAREN);
				break;
			case TK_CPAREN: 
				if (is_empty() || top() != TK_OPAREN) {
					printf("check_paren(%d, %d): bad expression\n", ii, jj);
					destroy_stack();
					return false;
				}
				pop();
				break;
			default:;
		}//end switch

		// 第二次检查
	  if ( option == 1 && p < q) {
			if (is_empty()) { // 若表达式是被括号包裹，那么不到最后一次永不会为空
				destroy_stack();
				return false;
			}
		}	
	}// end for

	if ( !is_empty() ) {
		//print_stack("not empty");	
		destroy_stack();
		//printf("check_paren2(%d, %d, %d): bad expression\n", ii, jj, option);
		return false;
	}
	if (option == 1) {
	}
	destroy_stack();
	return true;
}//end function
			

/* chuan - discard this method  */
// =============================================================
/*
int cal_expr() {
	int vals[nr_token];
	int idx1 = 0;
	int ops[nr_token];
	int idx2 = 0;
	int tmp = 0;
	
	// print tokens[] 
	for (int k = 0; k < nr_token; k++) {
		printf("%d: tokens[%d].type = %d, ", k, k, tokens[k].type);
		for (int j = 0; tokens[k].str[j] != '\0'; j++) {
				printf("tokens[%d].str = %c", k, tokens[k].str[j]);
		}
		printf("\n");
	}
	
	for (int i = 0 ;i < nr_token; i++){
		switch (tokens[i].type) {
			case TK_PLUS: 	ops[idx2++] = TK_PLUS; break;
			case TK_EQ:		break;    // need to do sth in future
			case TK_VAL:		vals[idx1++] = atoi(tokens[i].str); break;
			case TK_MINUS:  ops[idx2++]  = TK_MINUS; break;
			case TK_MUL:		ops[idx2++]  = TK_MUL;	 break;
			case TK_DIV:		ops[idx2++]  = TK_DIV;   break;
			case TK_OPAREN:													 break;
			case TK_CPAREN: { 
				idx2--;
				if (idx1 == 1) break;    // for extra parenthesis, eg,((1+4))

				if 			 (ops[idx2] == TK_PLUS) {
					tmp = vals[idx1-1] + vals[idx1-2];
				}else if (ops[idx2] == TK_MINUS) {
					tmp = vals[idx1-2] - vals[idx1-1];
				}else if (ops[idx2] == TK_MUL) {
					tmp = vals[idx1-2] * vals[idx1-1];
				}else if (ops[idx2] == TK_DIV) {
					tmp = vals[idx1-2] / vals[idx1-1];
				}else {
					printf("cal_expr: unknown ops[%d]= %d\n", idx2, ops[idx2+1]);
			   	assert(0);
				}
				// pop two vals 
				idx1 -= 2;
				// push the result
				vals[idx1++] = tmp;

				break;
			} // case TK_CPAREN end
				
			default:	{
								printf("cal_expr: unknown tokens[%d].type = %d\n", 
													i, tokens[i].type);
								assert(0);
							}
		} // switch tokes.type[] end

	} // for end

	if (idx1 != 1){
		printf("cal_expr: vals[] not end. idx1=%d, vals[0] = %d\n", idx1, vals[0]);
		assert(0);
	}
	
	printf("cal_expr: idx1=%d, vals[0] = %d\n", idx1, vals[0]);
	return vals[0];
}
// ================discard end ============================================
*/
