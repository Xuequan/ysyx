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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <math.h>
#include <memory/vaddr.h>

word_t eval(int p, int q); 
int find_main_op(int p, int q);
static bool check_parentheses(int p, int q, int option);
void assign_tokens_type(int type, int *index);
void transfer_tokens(int tokens_length);
void print_tokens(int nr_token);
word_t get_mem_val(word_t address);

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_VAL,   // decimal number
	TK_PLUS,  // plus
	TK_SUB,	// minus 260
	TK_MUL,		// mul
	TK_DIV,		// div
	TK_OPAREN, // open parenthesis 263
	TK_CPAREN, // close parenthesis
	TK_NEWLINE, 
	TK_REG,   // register, like x0-x31
	TK_HEX,		// hexadecimal-number, like 0x, 0X

	/* now only support *address, not support *variable 
	** so no rules[] for this 
	*/
	TK_DEREF,  
						
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
	{"0[xX][0-9a-fA-F]{1,8}", TK_HEX},    // hexadecimal numbers, should be at the front of TK_VAL
	{"[0-9]+", TK_VAL},  				// decimal numbers
	{"\\-", TK_SUB},          // minus
	{"\\*", TK_MUL},					  // mul
	{"\\/", TK_DIV},					  // div
	{"\\(", TK_OPAREN},					// open parenthesis	
	{"\\)", TK_CPAREN},					// close parenthesis
	{"\\\n", TK_NEWLINE},        // newline
	{"x[0-9]{1,2}", TK_REG},     // register, eg, x0-x31
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

  for (i = 0; i < NR_REGEX; i++) {
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
  int i = 0;
  regmatch_t pmatch;
  nr_token = 0;
	//printf("make_token( %s )\n", e);
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
				printf("position = %d\n", position);

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
				if (substr_len > 32) {
					printf("token is too long.\n");
					assert(0);
				}
				/* copy the new token to a buffer token.str */
				strncpy(tokens[nr_token].str, substr_start, (size_t) substr_len);
				tokens[nr_token].str[substr_len] = '\0';	
			
				/*
				// print this token 
				char *tmp = tokens[nr_token].str;
				printf("%d: tokens[%d].str = %s\n", nr_token, nr_token, tmp);
				*/
				/* copy the new token to a buffer token.type */
				assign_tokens_type(rules[i].token_type, &nr_token);
				nr_token++;
        break;
      } // end if (regexec(&re[i]...) 
    } // end for ( ; i < NR_REGEX; ...)

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }//end while
	// nr_token is the last index of tokens[]
	nr_token -= 1;
	transfer_tokens(nr_token + 1);
	//print_tokens(nr_token + 1);
  return true;
}

/* 1> if tokens[].type = TK_REG (register,eg, x10)
** get it value and copy it to tokens[].str
** 3> if tokens[].type == TK_MUL, 
** check if it is TK_DEFER, if so, check the expr behind it
** and conver it to TK_VAL
*/ 
void transfer_tokens(int tokens_length) {
	word_t reg_val = 0;
	bool success = false;
	int i = 0;
	
	for(; i < tokens_length; i++) {
		if (tokens[i].type == TK_REG) {
			tokens[i].type = TK_VAL;  // transfer register to number
			reg_val = isa_reg_str2val(tokens[i].str, &success);
			if (success == false) {
				printf("isa_reg_str2val() falied\n");
				assert(0);
			}
			snprintf(tokens[i].str, sizeof(word_t), "%u", reg_val);
		} // end if(tokens[i].type...)
		
	/*
		if (tokens[i].type == TK_HEX) {
			// convert hexadeximal(eg, 0xf)  to dec (eg, 15)
			tokens[i].type = TK_VAL;  
			//int hex_val = strtol(tokens[i].str, NULL, 16);
			//snprintf(tokens[i].str, sizeof(int), "%d", hex_val);
		}
	*/
	} // end for(; i < ...)

	// check for TK_DEREF should be after all above(TK_REG, TK_HEX)
	for ( i = 0; i < tokens_length; i++) {
		if (tokens[i].type == TK_MUL && 
			( i == 0 ||
				tokens[i-1].type == TK_MUL ||
				tokens[i-1].type == TK_SUB ||
				tokens[i-1].type == TK_PLUS ||
				tokens[i-1].type == TK_DIV ||
				tokens[i-1].type == TK_OPAREN 
			) ) {
			tokens[i].type = TK_DEREF;
			/*
			int follow_expr_val = eval_follow_expr(i);
			snprintf(tokens[i].str, sizeof(int), "%d", hex2dec_val);
			*/
		}// end if (tokens[i]...)
	} // end for ( i = 0...)
				
} // end function

/* tokens[i].type == TK_DEFER, eval_follow_expr will
** configure out the following <expr> 
*/
int eval_follow_expr(int i) {
	return 0;
}// end function


/* choose rules[].token_type and 
** assign it to tokens[].type
*/ 
void assign_tokens_type(int type, int *index) {
	//switch (rules[i].token_type) {
	switch (type) {
		case TK_NOTYPE:    // if spaces, do not record
		case TK_NEWLINE:
			{
				//nr_token--; 
				(*index)--;
				break;				
			}

		case TK_PLUS:  case TK_EQ: 	case TK_VAL: 	
		case TK_SUB: case TK_MUL: case TK_DIV: 
		case TK_OPAREN: case TK_CPAREN:
		case TK_REG:    case TK_HEX:
			{ 
				//tokens[nr_token].type = rules[i].token_type;  
				//tokens[nr_token].type = type;  
				tokens[*index].type = type;  
				break;
			}
					
		default: 
			{ printf("make_token(): unknown token_type \"%d\"\n", 
														type);
				assert(0);
      }

	} //end switch
} // end function

void print_tokens(int length) {
	printf("print_tokens : \n");

	for(int i = 0; i < length; i++) {
		printf("  tokens[%d].type = %d, tokens[%d].str = %s\n", i, tokens[i].type, i, tokens[i].str);
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

/* chuan, p < q
** only the final result will be word_t
** intermediate result is int 
*/
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
			if (tokens[p].type == TK_HEX) {
				return (word_t)strtol(tokens[p].str, NULL, 16);
			} else {
				return (word_t)atoi(tokens[p].str);
			}
	} else if (check_parentheses(p, q, 1) == true) {
		/* The expression is surrounded by a matched pair parentheses.
		 * If that is the case. just throw away the parentheses exper.
		 */
		return eval(p + 1, q - 1);
	} else if (check_parentheses(p, q, 0) == true) {
		op = find_main_op(p, q);
		//printf("eval(%d, %d), main op = %d\n", p, q, op);
		if (tokens[op].type != TK_DEREF ) {
			val1 = (int)eval(p, op - 1);
			val2 = (int)eval(op + 1, q);
		
			switch (tokens[op].type) {
				case TK_PLUS: return val1 + val2;
				case TK_SUB: return val1 - val2;
				case TK_MUL: return val1 * val2;
				case TK_DIV: 
					if (val2 == 0) {
						printf("div by zero error\n");
						assert(0);
					}
					return (word_t) (val1 / val2);
			
				default: assert(0);
			}//end switch
		} else {
			//printf("eval(%d, %d)\n", op + 1, q);
			word_t val3 = eval(op + 1, q);
			//printf("val3 = %#x\n", val3);
			return get_mem_val(val3);
		} // end if (tokens[op].type != TK_DEREF) 
	} else if (check_parentheses(p, q, 0) == false) {
		printf("parentheses are not matched. Plese input again\n");
		assert(0);
	} else {
		printf("eval(): unknown error!\n");
		assert(0);
	}
}

/* get_mem_val(int address) 
** Now only support *address;
** get the val from the address 
** Do not support *variable!!!
*/
word_t get_mem_val(word_t address) {
	// this maybe wrong!!!
	printf("get_mem_val(%u)\n", address);
	return vaddr_read(address, sizeof(word_t));;
} // end function

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
		if (tokens[i].type == TK_PLUS || tokens[i].type == TK_SUB
		 || tokens[i].type == TK_MUL  || tokens[i].type == TK_DIV) {
			if( (check_parentheses(p, i-1, 0) == true) && 
					(check_parentheses(i + 1, q, 0) == true) ) {
					index[cnt] = i;
					cnt++;
			}
		} // end if (tokens[i].type == TK_PLUS...)
		
		if (tokens[i].type == TK_DEREF) {
			if (check_parentheses(i + 1, q, 0) == true ) {
				index[cnt] = i;
				cnt++;
			}	
		} // end if (tokens[i].type == TK_DEREF...)
	}//end fo

	// figure out which is the main operator
	int mul_div_index = 0;
	int plus_sub_index = 0;
	int defer_index = 0;

	if (cnt == 1) {
		return index[0];
	} else {
		for ( i = 0; i < cnt; i++) {
			if (tokens[index[i]].type == TK_PLUS || 
					tokens[index[i]].type == TK_SUB) {
				plus_sub_index = index[i];
			} else if (tokens[index[i]].type == TK_DEREF) { 
				defer_index = index[i];
			} else {
				mul_div_index = index[i];
			} 
		}//end for
	}// end if-else
	
	if (plus_sub_index != 0) {
		return plus_sub_index;
	} else if (mul_div_index != 0) {
		return mul_div_index;
	} else {
		return defer_index;
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
	return true;
}//end function

