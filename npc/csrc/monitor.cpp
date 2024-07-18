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

#include <elf.h>
#include <getopt.h>
#include <time.h>
#include "common2.h"
#include <cstdlib>

uint8_t* guest_to_host(paddr_t);
uint8_t* flash_guest_to_host(paddr_t);
void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);
void init_elf();
void init_flash();

long test_size = 0;

void init_rand() {
	srand(time(0) );
}

FILE *log_fp = NULL;
void init_log(const char *log_file) {
	log_fp = stdout;
	if (log_file != NULL) {
		FILE *fp = fopen(log_file, "w");
		Assert(fp, "Cannot open '%s'", log_file);
		log_fp = fp;
	}
	Log("Log is written to %s", log_file ? log_file : "stdout");
}
#define CONFIG_TRACE 1
static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s!\n", ANSI_FMT("NPC", ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}


void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;
char *elf_file = NULL;

// load npc/tests/char-test.bin to flash space
static long load_test() {
	char *test_file = "/home/chuan/ysyx-workbench/npc/tests/char-test.bin";
	FILE *fp = fopen(test_file, "rb");
	Assert(fp, "load_test(): Can not open '%s'", test_file);

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);

	Log("The test file is %s, size = %ld", test_file, size);
	
	fseek(fp, 0, SEEK_SET);
	int ret = fread(flash_guest_to_host(FLASH_BASE), size, 1, fp);
	assert(ret == 1);

	fclose(fp);
	return size;	
}
// load_img() 将 img 文件装载到内存中某处；
// 只是方便在初始化 DiffTest 时，将其复制给 NMEU 对应的取指起始处
static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

	printf("image file : %s\n", img_file);

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "load_img(): Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
	// 将 image 读到 RESET_VECTOR (0x8000_0000) 对应的电脑的内存
	// 处，即 pmem 处；
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);
	/*
	for(int i = 0; i < size/4; i++)
		printf("%d: %#x\n", i, *(uint32_t *)(guest_to_host(RESET_VECTOR) + i));
	*/

  fclose(fp);
	return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:f:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'f': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-f,--elf=FILE           output log to log?\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  //init_rand();

  /* Open the log file. */
  init_log(log_file);

	/* Read ELF file and get strtab & symtab. */
  init_elf();

  /* Initialize memory. */
	// for test;
  init_mem();

  /* Initialize devices. */
  //IFDEF(CONFIG_DEVICE, init_device());

	/* init flash */
	init_flash();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

	test_size = load_test();
  /* Initialize differential testing. */
 	init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

	char str[] = "riscv32-pc-linux-gnu";
	init_disasm(str);

  /* Display welcome message. */
  welcome();
}
