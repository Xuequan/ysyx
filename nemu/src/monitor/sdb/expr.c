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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_VAL,   // decimal number
	TK_PLUS,  // plus
	TK_MINUS,	// minus
	TK_MUL,		// mul
	TK_DIV,		// div
	TK_OPAREN, // open parenthesis
	TK_CPAREN, // close parenthesis
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

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

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
			
				/* print tokens[].str */	
				char * tmp = tokens[nr_token].str;
				printf("%d: tokens[%d].str = %s\n", nr_token, nr_token, tmp);

        switch (rules[i].token_type) {
					case TK_NOTYPE: nr_token--; break;				// if spaces, do not record
					case TK_PLUS: tokens[nr_token].type = TK_PLUS;
												break;
					case TK_EQ: 	tokens[nr_token].type = TK_EQ;  
												break;
					case TK_VAL: 	tokens[nr_token].type = TK_VAL;  
												break;
					case TK_MINUS: tokens[nr_token].type = TK_MINUS;  
												break;
					case TK_MUL: tokens[nr_token].type = TK_MUL;  
												break;
					case TK_DIV: tokens[nr_token].type = TK_DIV;  
												break;
					case TK_OPAREN: tokens[nr_token].type = TK_OPAREN;  
												break;
					case TK_CPAREN: tokens[nr_token].type = TK_CPAREN;  
												break;
				
          default: printf("make_token(): unknown token_type \"%d\"\n", 
																				rules[i].token_type);
									 assert(0);
        }
				nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


int cal_expr();

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return (word_t)cal_expr();
}

int cal_expr() {
	int i = 0;
	int vals[nr_token];
	int idx1 = 0;
	int ops[nr_token];
	int idx2 = 0;
	int tmp = 0;
	
	/* print tokens[] */
	for (int k = 0; k < nr_token; k++) {
		printf("%d: tokens[%d].type = %d, ", k, k, tokens[k].type);
		for (int j = 0; tokens[k].str[j] != '\0'; j++) {
				printf("tokens[%d].str = %c", k, tokens[k].str[j]);
		}
		printf("\n");
	}
	
	printf("start: idx1=%d, idx2=%d\n", idx1, idx2);
	for ( ;i < nr_token; i++){
		switch (tokens[i].type) {
			case TK_PLUS: 	ops[idx2++] = TK_PLUS; break;
			case TK_EQ:		break;    // need to do sth in future
			case TK_VAL:		vals[idx1++] = atoi(tokens[i].str); break;
			case TK_MINUS:  ops[idx2++]  = TK_MINUS; break;
			case TK_MUL:		ops[idx2++]  = TK_MUL;	 break;
			case TK_DIV:		ops[idx2++]  = TK_DIV;   break;
			case TK_OPAREN:													 break;
			case TK_CPAREN: { 
				switch (ops[idx2--]) {
					case TK_PLUS:		tmp = vals[idx1-1] + vals[idx1-2]; idx1 -= 2; 
													vals[idx1++] = tmp; break;
					case TK_MINUS:	tmp = vals[idx1-1] + vals[idx1-2]; idx1 -= 2; 	
													vals[idx1++] = tmp; break;
					case TK_MUL:		tmp = vals[idx1-1] * vals[idx1-2]; idx1 -= 2; 	
													vals[idx1++] = tmp; break;
					case TK_DIV:		tmp = vals[idx1-1] / vals[idx1-2]; idx1 -= 2; 	
													vals[idx1++] = tmp; break;
					default:	printf("cal_expr: unknown ops[]\n");
										assert(0);
					}
				}
			default:	printf("cal_expr: unknown tokens[%d].type = %d\n", 
													i, tokens[i].type);
								assert(0);
		}
		
		printf("%d: idx1=%d, idx2=%d\n", i,  idx1, idx2);
	}

	if (idx1 != 2){
		printf("cal_expr: vals[] not end. idx1=%d, vals[0] = %d\n", idx1, vals[0]);
		assert(0);
	}
	
	printf("cal_expr: idx1=%d, vals[0] = %d\n", idx1, vals[0]);
	return vals[0];
}
				
				
					
	
