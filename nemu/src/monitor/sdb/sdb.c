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

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

/* chuan, start*/
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

	}else if (*args == 'w') {  //info watchpoint 
		printf("'info w' needs added, please wait.\n");

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
	printf("number = %d, memory address = %xd\n", (int)atof(num), (int)atof(addr));	

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
	{"info", "Show all register information or watchpoints, now only 'info w' and 'into r' two commands", cmd_info},
	{"x", "Show memory content, fromat 'x N EXPR'", cmd_x},
	/*
	{"p", "Print value of expression", cmd_p},
	{"w", "Set watchpoint", cmd_w},
	{"d", "Delete watchpoint", cmd_d},
	*/

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
		/* chuan: eg, p abd
			 then args points to "abd"
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
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
