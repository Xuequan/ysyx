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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

/*chuan */
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

/* should stop at breakpoints if exist */
static int cmd_c(char *args) {
	// -1 transfer to a big uint64_t number
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

/* chuan, start*/
/* should stop at breakpoints if has */
static int cmd_si(char *args) {
	/* chuan, if no number, set 1 */
	if (args == NULL) 
		cpu_exec(1);
	else
  	cpu_exec(*args);
  return 0;
}

static int cmd_info(char *args) {
	if (args == NULL) {
		printf("Please input arguments 'r' or 'w' of command 'info'.\n");
	}else if (*args == 'r') {
		// nemu/src/isa/$ISA/reg.c
		isa_reg_display();

	}else if (*args == 'w') {  // "info w" info watchpoint 
		print_wp();
	}else{
		printf("Unknown command argument, please input 'info r' or 'info w' again.\n");
	}

	return 0;
}

static int cmd_x(char *args) {
	char *str = args;
	char *num = NULL;
	char *addr = NULL;
	int  whitespace_num = 0;

	if (args == NULL) {
		printf("Please input arguments, for example, 'x 10 0x8000000'\n");
		return 0;
	}else {
		// 1. parse args
		// for (; *str++ != '\0'; ) { // This doesnt work, why?
		for (; *str != '\0'; str++) {   
			if (*str == ' ' && whitespace_num == 0) {
				num = args;
				*str = '\0';	
				whitespace_num = 1;
			}else if (*str == ' ' && whitespace_num == 1) {
				continue;
			}else if (*str != ' ' && whitespace_num == 1) {
				addr = str;
				break;
			}
		}	
	}
	/* 2. show data */
	int print_num = (int)atof(num);
	vaddr_t print_addr = (vaddr_t)atof(addr); 
	//printf("number = %d, memory address = %#x\n", (int)atof(num), (int)atof(addr));	
	word_t data;
	for (int i = 0; i < print_num; i++) {
		data = vaddr_read(print_addr, 4);
		printf("%#x		%#x\n", print_addr, data); 
		print_addr = print_addr + sizeof(word_t);
	}

	return 0;
}

static int cmd_w(char *args) {
	if (args == NULL) {
		printf("Please input arguments, for example, 'w *0x80000000'\n");
		return 0;
	}
	// check if input args is valid 
	bool success = false;
	word_t expr_result = expr(args, &success); 
	if (success == false) {
		printf("cmd_w() : cannot recognize \"%s\".\n", args);
		return 0;
	}
	// check if always had a same wp
	if (check_repeated_wp(args) == true) {
		printf("Already had a same one.\n");
		return 0;
	}	
	WP *wp = new_wp();
	if (wp == NULL) {
		printf("cmd_w(): cannot get a new watchpoint\n");
		return 0;
	}
	strcpy(wp->expr, args);
	wp->val = expr_result;
	printf("Watchpoint %d: %s, value = %u\n", wp->NO, wp->expr, wp->val);
	
	return 0;
}

#define TEST_LENGTH (65536 + 11)

static int cmd_p(char *args) {

	if (args == NULL) {    // for test only
		// test use nemu/tools/gen-expr
		FILE *fp = fopen("/home/chuan/ysyx-workbench/nemu/tools/gen-expr/input", "r");
		assert(fp != NULL);

		char buf[TEST_LENGTH] = {};

		while(fgets(buf, TEST_LENGTH, fp) != NULL) {
			char *buf_end = buf + strlen(buf);
			char *test_result_buf = strtok(buf, " ");
			if (test_result_buf == NULL) {
				printf("Didn't get the gen-expr result\n");
				assert(0);
			}

			char *expr_buf = test_result_buf + strlen(test_result_buf) + 1;
			if (expr_buf > buf_end) { assert(0); }
			if (expr_buf == NULL)   { assert(0); }

			bool success = false;

			printf("expr = %s\n", expr_buf);
			
			// 自己函数的计算结果
			word_t expr_result = expr(expr_buf, &success);
			if (success == false) {
				printf("Calculate \"expr = %s\" failed\n", expr_buf);
				assert(0);
			}
			// test_result 是电脑计算的结果
			word_t test_result = (word_t) atol(test_result_buf);
			if ( expr_result != test_result ) 
			{
				printf("expr_result = %u, test_result = %u\n", 
											expr_result, test_result);
				assert(0);
			} 
		} // end while
		fclose(fp);
		
	}else {
			bool success = false;
			word_t expr_result = expr(args, &success); 
			if (success == false) {
				printf("cmd_p(): cannot recognize \"%s\".\n", args);
				return 0;
			} else {
				printf("%u\n", expr_result);
			}
	}
	return 0;
}

/* just a copy of cmd_p() */
static int cmd_px(char *args) {
	bool success = false;
	word_t expr_result = expr(args, &success); 
	if (success == false) {
		printf("cmd_px(): cannot recognize \"%s\".\n", args);
	} else {
		printf("%#x\n", expr_result);
	}
	return 0;
}

static int cmd_d(char *args) {
	free_wp(atoi(args) );	
  return 0;
}
/* chuan, end */

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
	{"si", "Step one instruction exactly", cmd_si},
	{"info", "Show all register information or watchpoints, now only 'info w' and 'into r'", cmd_info},
	{"x", "Show memory content, fromat 'x N EXPR'", cmd_x},
	{"p", "Print value of expression", cmd_p},
	{"p/x", "Print value of expression in hexadecimal fomat", cmd_px},
	{"w", "Set watchpoint, eg 'w *0x80000000'", cmd_w},
	{"d", "Delete watchpoint, eg 'd N'", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }
#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }
    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }

  } // end for (char *str ...)
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

