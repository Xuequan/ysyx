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
#include <cassert>
#include <cmath>
#include <stack>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <cstring>
#include "arch.h"
#include "common2.h"
#include <cstdio>
//#include "sim.h"

using namespace std;
uint32_t get_pc_from_top();
word_t vaddr_read(vaddr_t, int);
uint8_t* guest_to_host(paddr_t);
/* below function in this file */
word_t expr(char *e, bool *success);
void init_regex();
static bool make_token(char *e);
static int str2num(int index);
static word_t eval(int p, int q); 
static int find_main_op(int p, int q);
static bool check_parentheses(int p, int q);
static bool check_paren_valid(int p, int q);
static void assign_tokens_type(int type, int *index);
static void check_tokens_type(int tokens_length);
static word_t get_mem_val(word_t address);
//static void print_tokens(int nr_token);
static word_t check_expr(int length);

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
	TK_REG,
	TK_HEX,		// hexadecimal-number, like 0x, 0X

	/* now only support *address, not support *variable */
	TK_DEREF,  
	TK_NEGVAL,  // negative value
	TK_LESS_EQ,  // 270
	TK_LOG_AND,						
	TK_PC,      // only for $PC
};

/* Attention: NO reguler rules for TK_DEFER & TK_NEGVAL */
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

	{"0[xX][0-9a-fA-F]+", TK_HEX},    // hexadecimal numbers, should be at the front of TK_VAL
	{"[0-9]+", TK_VAL},  				// decimal numbers
	{"\\-", TK_SUB},            // minus
	{"\\*", TK_MUL},					  // mul
	{"\\/", TK_DIV},					  // div
	{"\\(", TK_OPAREN},					// open parenthesis	
	{"\\)", TK_CPAREN},					// close parenthesis
	{"\\\n", TK_NEWLINE},       // newline
	{"\\$[pP][cC]", TK_PC},			// $pc, should before TK_REG
	{"\\$[$rsgta][0-9ap][01]?", TK_REG},  // regrister, eg $t0
	{"<=", TK_LESS_EQ},          // <=
	{"&&", TK_LOG_AND},          // &&
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

/* get the tokens from input check_token_type(),expr
** 1. scan input expr, try all rules one by one;
** 2. when recognized, skip TK_NEWLINE & TK_NOTYPE, and copy it to tokens[];
** 3. After having recognized all tokens, check whether if TK_DEFER & TK_NEGVAL;
*/
static bool make_token(char *e) {
  int position = 0;
  int i = 0;
  regmatch_t pmatch;
  nr_token = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
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
				if (substr_len > 32) {
					printf("token is too long.\n");
					assert(0);
				}
				/* copy the new token to token.str */
				strncpy(tokens[nr_token].str, substr_start, (size_t) substr_len);
				tokens[nr_token].str[substr_len] = '\0';	
				/* assign the new type to token.type 
				 * skip TK_NOTYPE and TK_NEWLINE */
				assign_tokens_type(rules[i].token_type, &nr_token);
				nr_token++;
        break;  // jump out from "for", not "while"
      } // end if (regexec(&re[i]...) 
    } // end for ( ; i < NR_REGEX; ...)

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }//end while

	/* nr_token is the last index of tokens[]
	** not the length of tokens[] */
	nr_token -= 1;

	// check if TK_DEFER & TK_NEGVAL
	check_tokens_type(nr_token + 1);

//	print_tokens(nr_token + 1);
  return true;
}

/* help to check if tokens' type is TK_NEGVAL or TK_DEREF */
static bool is_certain_type(int type) {
	return type == TK_MUL ||
				 type == TK_SUB ||
				 type == TK_PLUS ||
				 type == TK_DIV ||
				 type == TK_OPAREN ||
				 type == TK_EQ ||
				 type == TK_LESS_EQ ||
				 type == TK_LOG_AND;
}

/* handle TK_REG, TK_PC, TK_SUB, TK_MUL 
** for TK_REG & TK_PC, get its value and copy to token.str
** for TK_MUL& TK_SUB, check if token is TK_DEFER, TK_NEGVAL
*/ 
static void check_tokens_type(int tokens_length) {
	word_t reg_val = 0;
	bool success = false;
	int i = 0;
	
	for(; i < tokens_length; i++) {
		if (tokens[i].type == TK_REG) {
			char* ptr = tokens[i].str;
			ptr++;
			reg_val = isa_reg_str2val(ptr, &success);
			if (success == false) {
				printf("isa_reg_str2val() falied\n");
				assert(0);
			}
			/*
			memcpy(tokens[i].str, &reg_val, sizeof(word_t));
			tokens[i].str[sizeof(word_t)] = '\0';
			*/
			sprintf(tokens[i].str, "%#x", reg_val);
		} 
	}
		
	for(i = 0; i < tokens_length; i++) {
		if (tokens[i].type == TK_PC) {
			sprintf(tokens[i].str, "%#x", get_pc_from_top());
		} 
	} 

	// check for TK_DEREF; should be after all above(TK_REG, TK_HEX)
	for ( i = 0; i < tokens_length; i++) {
		if (tokens[i].type == TK_MUL && 
			( i == 0 || is_certain_type(tokens[i-1].type) ) ) {
			tokens[i].type = TK_DEREF;
		}
	} 
				
	// check for negative number
	for ( i = 0; i < tokens_length; i++) {
		if (tokens[i].type == TK_SUB && 
			( i == 0 || is_certain_type(tokens[i-1].type) ) ) {
			tokens[i].type = TK_NEGVAL;
		}
	} 
} // end function

/* choose rules[].token_type and 
** assign it to tokens[].type
** delete TK_NOTYPE, TK_NEWLINE
*/ 
static void assign_tokens_type(int type, int *index) {
	switch (type) {
		case TK_NOTYPE:    // if spaces, do not record
		case TK_NEWLINE:
			{
				(*index)--;
				break;				
			}
		case TK_PLUS:   case TK_EQ: 	case TK_VAL: 	
		case TK_SUB:    case TK_MUL:  case TK_DIV: 
		case TK_OPAREN: case TK_CPAREN:
    case TK_HEX:		case TK_REG:
		case TK_LESS_EQ: case TK_LOG_AND:
		case TK_PC:			 
			{ 
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


/* print_tokens help function */
/*
static char* print_help(int no) {
	switch (no) {
		case 256: return "TK_NOTYPE";
		case 257: return "TK_EQ";
		case 258: return "TK_VAL";
		case 259: return "TK_PLUS";
		case 260: return "TK_SUB";
		case 261: return "TK_MUL";
		case 262: return "TK_DIV";
		case 263: return "TK_OPAREN";
		case 264: return "TK_CPAREN";
		case 265: return "TK_NEWLINE";
		case 266: return "TK_REG";
		case 267: return "TK_HEX";
		case 268: return "TK_DEREF";
		case 269: return "TK_NEGVAL";
		case 270: return "TK_LESS_EQ";
		case 271: return "TK_LOG_AND";
		case 272: return "TK_PC";
		default:  return "error";
	}
}
static void print_tokens(int length) {
	printf("=======================\n");
	printf("print tokens, total %d tokens: \n", length);

	for(int i = 0; i < length; i++) {
		printf("  tokens[%d].type = %-12s tokens[%d].str = ", 
					i, print_help(tokens[i].type), i);

		for (char *ptr = tokens[i].str; *ptr != '\0'; ptr++){
			printf("%c", *ptr);
		}
		printf("\n");
	}
	printf("=======================\n");
}

*/
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;

	word_t reference_result = check_expr(nr_token + 1);
	word_t eval_result = eval(0, nr_token);
	if ( (reference_result == 0) ? 1 : reference_result != eval_result ) 
	{
		//printf("Please check, calculate error\n");
		;
	}
	return eval_result;
}

/* 根据tokens[index].type 的不同，
** 对应check_token_type(),用不同的方法将 tokens[].str 转化为数值
*/
static int str2num(int index) {
	switch (tokens[index].type) {
		case TK_PC: case TK_REG: 
				//return *(int*)tokens[index].str;
				return (int) strtol(tokens[index].str, NULL, 16);

		case TK_HEX: 
				return (int) strtol(tokens[index].str, NULL, 16);
		
		default: 
				return atoi(tokens[index].str);
	}
}
/*
static bool check_ov (int val1, int val2, int op) {
	
}
*/

static int eval_int_ret (int p, int q) {
	if (p > q) {
		printf("eval(): bad expression\n");
		assert(0);
	} else if (p == q) {
		/* Single token, should be a number */
		return str2num(p);
	} else if (check_parentheses(p, q) == true) {
		/* The expression is surrounded by a matched pair parentheses.
		 * If that is the case. just throw away the parentheses exper.
		 */
		return eval(p + 1, q - 1);
	} else {
		int op = find_main_op(p, q);

		if (tokens[op].type == TK_DEREF) {
			int val3 = eval(op + 1, q);
			return (int)get_mem_val(val3);

		} else if (tokens[op].type == TK_NEGVAL) {
			int val3 = eval(op + 1, q);
			return 0 - val3;

		} else {
			int val1 = eval(p, op - 1);
			int val2 = eval(op + 1, q);
		
			switch (tokens[op].type) {
				case TK_PLUS: 
					return val1 + val2;
				case TK_SUB: 
					return val1 - val2;
				case TK_MUL: 
					return val1 * val2;
				case TK_DIV: 
					{
						if (val2 == 0) {
							assert(0);
						}
						return (val1 / val2);
					}
				case TK_EQ:  
					if (val1 == val2) {
						return 1;
					} else {
						return 0;
					}
				case TK_LESS_EQ:  
					if (val1 <= val2) {
						return 1;
					} else {
						return 0;
					}
				case TK_LOG_AND:  
					if (val1 && val2) {
						return 1;
					} else {
						return 0;
					}
				default: assert(0);
			}//end switch
		} // end if (tokens[op].type ...) 
	}
}

/* only the final result will be word_t
** intermediate result is int 
*/
static word_t eval (int p, int q) {
	/* check overflow */	
	int temp = eval_int_ret(p, q);
	/*
	if (temp < 0) {
		printf("Please note: maybe overflow!\n");
	}
	*/
	return (word_t)temp;
}

/* get_mem_val(int address) 
** Now only support *address;
** get the val from the address 
** Do not support *variable!!!
*/
static word_t get_mem_val(word_t address) {
	return vaddr_read(address, sizeof(word_t));
} 

/* find the position of main operator
** now support +, -, *, /, *(defer), ==, &&, <=
*/
static int find_main_op(int p, int q) {
	int i = 0;
	int cnt = 0;
	int index[q - p];
	
	// find all operators between [p, q]
	for(i = p; i <= q; i++) {
		if (tokens[i].type == TK_PLUS || tokens[i].type == TK_SUB
		 || tokens[i].type == TK_MUL  || tokens[i].type == TK_DIV
		 || tokens[i].type == TK_EQ   || tokens[i].type == TK_LESS_EQ
		 || tokens[i].type == TK_LOG_AND) 
		{ 
			/* 还要确保该op两边的括号要成对，不然该op在括号里 */
	  	if( (check_paren_valid(p, i-1) == true) &&
          (check_paren_valid(i + 1, q) == true) ) 
			{
          index[cnt] = i;
          cnt++;
      }       
    } // end if (tokens[i].type == TK_PLUS...)
        
    if (tokens[i].type == TK_DEREF || 
				tokens[i].type == TK_NEGVAL ) 
		{
      if (check_paren_valid(i + 1, q) == true ) 
			{  
        index[cnt] = i;
        cnt++;
      }     
    } // end if (tokens[i].type == TK_DEREF...)
	}//end for

	// figure out which is the main operator
	int mul_div_index = 0;
	int plus_sub_index = 0;
	int defer_index = 0;
	int eq_index = 0;
	int less_eq_index = 0;
	int log_and_index = 0;
	int negval_index = 0;

	if (cnt == 1) {
		return index[0];
	} else {
		for ( i = 0; i < cnt; i++) {
			if (tokens[index[i]].type == TK_PLUS || 
					tokens[index[i]].type == TK_SUB) {
				plus_sub_index = index[i];
			} else if (tokens[index[i]].type == TK_DEREF) { 
				defer_index = index[i];
			} else if (tokens[index[i]].type == TK_NEGVAL) { 
				negval_index = index[i];
			} else if (tokens[index[i]].type == TK_EQ) { 
				eq_index = index[i];
			} else if (tokens[index[i]].type == TK_LESS_EQ) { 
				less_eq_index = index[i];
			} else if (tokens[index[i]].type == TK_LOG_AND) { 
				log_and_index = index[i];
			} else {
				mul_div_index = index[i];
			} 
		}//end for
	}// end if-else
	
	// 按优先级来选择
	if (log_and_index != 0) {       // &&
	 	return log_and_index;
	} else if (eq_index != 0) {       // ==
	 	return eq_index;
	} else if (less_eq_index != 0) {   // <= 
		return less_eq_index;
	} else if (plus_sub_index != 0) {   // +, -
		return plus_sub_index;
	} else if (mul_div_index != 0) {    // *
		return mul_div_index;							
	} else if (defer_index != 0) {     // *0x80000000 (deference)
		return defer_index;
	} else {				// -8 (negative val)
		return negval_index;
	}
}//end function

/* 检查表达式里的括号是否是成对出现
** 注意：没有括号也会返回 true
*/
static bool check_paren_valid(int p, int q) {
	assert(p <= q);

	stack <int> stack;
	/* 遍历; 遇到“(" 则入栈；遇到“）” 则出栈 */
	for(; p <= q; p++) {
		switch(tokens[p].type) {
			case TK_OPAREN: 
				stack.push(TK_OPAREN);
				break;

			case TK_CPAREN: 
				if ( stack.empty() ) {
					//printf("check_paren_valid(%d, %d): bad expression\n", p, q);
					return false;
				}else{
					stack.pop();
					break;
				} 
			default:;
		}//end switch
	}// end for

	if ( !stack.empty() ) {
		return false;
	}

	return true;
}

/* 检查表达式是否被一对括号包裹，以及表达式里的括号是否成对 */
static bool check_parentheses(int p, int q) { 

	/* 首先先看一头一尾是否是一对括号 */
	if (tokens[p].type != TK_OPAREN || 
				tokens[q].type != TK_CPAREN )  
		return false;

	/* 再检查括号里头的表达式的括号是否合法 */
	if ((check_paren_valid(p + 1, q - 1)) == false) 
		return false;

	return true;
}//end function

/* --------------------------------------------------- */
static char code_format[] =  
"#include <stdio.h>\n"
"int main() { \n"
"  unsigned result = %s; \n"
"  printf(\"%%u\", result); \n"
"  return 0; \n"
"} \n";

/* from nemu/tools/gen-expr/gen-expr.c */
word_t eval_from_tool(char expr[], int length) {

  char eval_code[length + 128];
 	for (int i = 0; i < length + 128; i++) {
		eval_code[i] = '\0';
	} 

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
  return (word_t)result;
}


static word_t check_expr(int length) {
	int i = 0;
	int m = 0;
	char buf[32 * length];
	for ( ; i < length; i++) {
		if (tokens[i].type == TK_DEREF) 
			return 0;
		for (int j = 0; j < (int)strlen(tokens[i].str); j++) {
			buf[m] = tokens[i].str[j];
			m++;
		}		
		buf[m] = ' ';
		m++;
	}
	buf[m] = '\0';
	int len = (int)strlen(buf);
	word_t ret = eval_from_tool(buf, len);
	//printf("buf = %s, value = %u\n", buf, ret);
	return ret;
}
