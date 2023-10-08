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

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

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
	time_t t;
	int num = 0;

	srand((unsigned) time(&t));
	num = rand();

	// add a num to the tail of buf[65535]
	char temp[RAND_MAX];
	uint32_t len = buf_length();	
	snprintf(temp, sizeof(temp), "%d", num);
	for (; temp[i] != '/0'; i++) {
		buf[len + i] = temp[i];
	}
	buf[len + i] = '\0';
}

void gen(char c) {
	uint32_t len = buf_length();
	buf[len] = c;
	buf[len + 1] = '\0';
}

void gen_rand_op(void) {
	uint32_t len = buf_length();
	char op[] = "+-*/";
	uint32_t index = choose(5);
	gen(op[index]);
}

static void gen_rand_expr() {
	time_t t;
	srand((unsigned) time(&t));
  buf[0] = '\0';
	switch(choose(3)) {
		case 0: gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); 
						 gen_rand_op();
						 gen_rand_expr();
						 break;
	}
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

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
  return 0;
}
