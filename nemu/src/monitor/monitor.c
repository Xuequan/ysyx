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
#include <memory/paddr.h>
#include <elf.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static char *elf_file = NULL;
static MUXDEF(CONFIG_RV64, Elf64_Ehdr, Elf32_Ehdr) ehdr;

typedef struct symtab_arg {
	uint32_t entsize;   // symtab every entry size
	uint32_t entnum;    // symtab number of entries
	// symtab offset in ELF file
	MUXDEF(CONFIG_RV64, Elf64_Off, Elf32_Off) off;
} SYMTAB; 

SYMTAB symtab;

typedef struct strtab_arg {
	// strtab offset in ELF file
	MUXDEF(CONFIG_RV64, Elf64_Off, Elf32_Off) off;
	// strtab total size
	MUXDEF(CONFIG_RV64, uint64_t, uint32_t) size;
} STRTAB; 
	
STRTAB strtab;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "load_img(): Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}


static bool check_elf() {
	// check elf_file is a ELF file
	if ( !(ehdr.e_ident[EI_MAG0] == ELFMAG0 && 
			ehdr.e_ident[EI_MAG1] == ELFMAG1 && 
			ehdr.e_ident[EI_MAG2] == ELFMAG2 && 
			ehdr.e_ident[EI_MAG3] == ELFMAG3) ) {
		printf("check_elf(): file '%s' is not a ELF file.\n", elf_file);
		return false;
	}
	// check if elf_file is 32-bit
	if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
		printf("check_elf(): file '%s' is not a ELF32.\n", elf_file);
		return false;
	}
	// check if elf_file is a executable file or shared object
	// if not, st_value is not a virtual address
	if (ehdr.e_type != ET_EXEC && ehdr.e_type != ET_DYN) {
		printf("check_elf(): file '%s' is not a executable file or shared object.\n", elf_file);
		return false;
	}

	return true;
}


/* open ELF file and get strtab & symtab */
static void init_elf() {
	FILE *fp = fopen(elf_file, "r");
	if (fp == NULL) {
		printf("init_elf(): Can not open '%s'\n", elf_file);
		return;
	}

	/* 1. get Ehdr and ELF file check */
	if ( fread(&ehdr, sizeof(ehdr), 1, fp) == 0) {
		printf("init_elf(): Cannot read ElfN_Ehdr.\n");
		fclose(fp);
		return;
	}

	if (check_elf() == false) {
		fclose(fp);
		return;
	}
		
	/* 2. get section header table */
  MUXDEF(CONFIG_RV64, Elf64_Shdr, Elf32_Shdr) shdr[ehdr.e_shnum][ehdr.e_shentsize];
	// 定位到 section header table
	if (fseek(fp, ehdr.e_shoff, SEEK_SET) != 0) {
		printf("init_elf(): Unable to set section header table postion\n");
		fclose(fp);
		return;
	}
	for( int i = 0; i < ehdr.e_shnum; i++) {
		if (fread(shdr[i], ehdr.e_shentsize, 1, fp) == 0) {
			printf("init_elf(): Unable to read shdr\n");
			fclose(fp);
			return;
		}
	}
	
	/* 3. get symbol tables (symtab) */
	int symtab_idx = 0;
	for( ; symtab_idx < ehdr.e_shnum; symtab_idx++) {
		if (shdr[symtab_idx]->sh_type == SHT_SYMTAB) {
			break;
		}
	}
	if (symtab_idx == ehdr.e_shnum) {
		printf("init_elf(): Failed to get symtab in section header table\n");
		fclose(fp);
		return;
	}

	uint32_t symentnum = shdr[symtab_idx]->sh_size / 
											 shdr[symtab_idx]->sh_entsize;
	symtab.entsize = shdr[symtab_idx]->sh_entsize;
	symtab.entnum  = symentnum;
	symtab.off     = shdr[symtab_idx]->sh_offset; 

	/* 4. get string tables (strtab) */
	// 这里我取巧了，直接从 symtab 开始查看，因为 .shstrtab 的 TYPE 也是 STRTAB
	int strtab_idx = 0;
	for( strtab_idx = symtab_idx; strtab_idx < ehdr.e_shnum; strtab_idx++) {
		if (shdr[strtab_idx]->sh_type == SHT_STRTAB) {
			break;
		}
	}
	if (strtab_idx == ehdr.e_shnum) {
		printf("init_elf(): Failed to get strtab in section header table\n");
		fclose(fp);
		return;
	}

	strtab.off  = shdr[strtab_idx]->sh_offset;
	strtab.size = shdr[strtab_idx]->sh_size;

	fclose(fp);
	return;	
} // end function


/* get strtab from reading ELF file 
** FILE *elf_fp is the opening fd of ELF file 
** char *buf is the buf[strtab.size]
** return true if success, otherwise false.
*/
bool get_strtab(FILE *elf_fp, char* buf) {

	if (elf_fp == NULL) {
		FILE *elf_fp = fopen(elf_file, "r");
		if (elf_fp == NULL) {
			printf("get_strtab(): Can not open '%s'\n", elf_file);
			return false;
		}
	}
	// 定位到 strtab 的位置
	if (fseek(elf_fp, strtab.off, SEEK_SET) != 0) {
		printf("get_strtab(): Unable to set strtab postion\n");
		return false;
	}

	if ( fread(buf, strtab.size, 1, elf_fp) == 0) {
		printf("get_strtab(): Cannot get strtab\n");
		return false;
	}	
	return true;
}
		
// 测试，打印 strtab
void print_strtab(char buf[]) {

	printf("========== print strtab start =============\n");

	char *token;
	int i =0, j = 0;
	int num = 0;
	for( i = 0; i < strtab.size; ){
		token = buf + i;
		printf("%d: %s\n", num, token);	
		num++;
		for( j = i; j < strtab.size; ){
			if ( *(buf + j) != '\0')
				j++;
			else
				break;
		}
		i = j + 1;
	}
	printf("========== print strtab end =============\n");
}

/* get symtab from reading ELF file 
** FILE *elf_fp is the opening fd of ELF file 
** char *buf[][] is &symtab[symtab.entnum][symtab.entsize]
** return true if success, otherwise false.
*/
bool get_symtab(FILE *elf_fp, char*** buf) {

	if (elf_fp == NULL) {
		FILE *elf_fp = fopen(elf_file, "r");
		if (elf_fp == NULL) {
			printf("get_symtab(): Can not open '%s'\n", elf_file);
			return false;
		}
	}
	// 定位到 symtab
	if (fseek(elf_fp, symtab.off, SEEK_SET) != 0) {
		printf("get_symtab(): Unable to set symtab postion\n");
		fclose(elf_fp);
		return false;
	}

	for( int i = 0; i < symtab.entnum; i++) {
		if (fread(*buf[i], symtab.entsize, 1, elf_fp) == 0) {
			printf("get_symtab(): Unable to get symtab\n");
			fclose(elf_fp);
			return false;
		}
	}
	return true;
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
  init_rand();

  /* Open the log file. */
  init_log(log_file);

	/* Initialize ELF file. */
	init_elf(elf_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
