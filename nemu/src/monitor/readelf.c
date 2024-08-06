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
#include <elf.h>
#include <string.h>

extern char *elf_file;

//#define ENTRY_NUM 258
static struct {
	// actual symtab entry number
	word_t entnum;
	word_t entsize;
	//MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym) sym[ENTRY_NUM];
	// 记录 symtab 在ELF开始地址
	word_t addr;    
} symtab;

//#define STRTAB_SIZE 2014
//static char strtab[STRTAB_SIZE];	
static struct {
	word_t size;
	word_t addr;
} strtab;

/*
// for test only, print strtab
static void print_strtab(char buf[], int strtab_size) {
	printf("========== print strtab start =============\n");
	char *token;
	int i =0, j = 0;
	int num = 0;
	for( i = 0; i < strtab_size; ){
		token = buf + i;
		printf("%d: %s\n", num, token);	
		num++;
		for( j = i; j < strtab_size; ){
			if ( *(buf + j) != '\0')
				j++;
			else
				break;
		}
		i = j + 1;
	}
	printf("========== print strtab end =============\n");
}

static void print_strtab2(char buf[]) {
	printf("========== print2 strtab start =============\n");
	int i = 0;	
	char *ret;
	for( ; i < symtab.entnum; i++) {
		ret = buf + symtab.sym[i].st_name; 	
		printf("%d - %s\n", symtab.sym[i].st_name, ret);
	}//end-for
	printf("========== print2 strtab end =============\n");
}
*/

static FILE* open_elf() {
	if (elf_file == NULL) {
		printf("no input ELF file\n");
		return NULL;
	}
	FILE *fp = fopen(elf_file, "r");

	if (fp == NULL) {
		printf("Can not open '%s'\n", elf_file);
		return NULL;
	}
	return fp;
}

/* read from ELF file and get strtab & symtab */
void init_elf() {
	/*
	if (elf_file == NULL) {
		printf("init_elf(): no input ELF file\n");
		return;
	}
	FILE *fp = fopen(elf_file, "r");

	if (fp == NULL) {
		printf("init_elf(): Can not open '%s'\n", elf_file);
		return;
	}
	*/
	if (elf_file == NULL) {
		printf("No input ELF file\n");
		return;
	}

	FILE *fp = open_elf();
	assert(fp != NULL);

	/* 1. get Ehdr and ELF file check */
	MUXDEF(CONFIG_RV64, Elf64_Ehdr, Elf32_Ehdr) ehdr;

	if ( fread(&ehdr, sizeof(ehdr), 1, fp) == 0) {
		printf("init_elf(): Cannot read ElfN_Ehdr.\n");
		fclose(fp);
		return;
	}

	// check if elf_file is a ELF file
	if ( !(ehdr.e_ident[EI_MAG0] == ELFMAG0 && 
			ehdr.e_ident[EI_MAG1] == ELFMAG1 && 
			ehdr.e_ident[EI_MAG2] == ELFMAG2 && 
			ehdr.e_ident[EI_MAG3] == ELFMAG3) ) {
		printf("init_elf(): file '%s' is not a ELF file.\n", elf_file);
		return;
	}
	// check if elf_file is 32-bit
	if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
		printf("init_elf(): file '%s' is not a ELF32.\n", elf_file);
		return;
	}
	// check if elf_file is a executable file or shared object
	// if not, st_value is not a virtual address
	// then cannot find function name
	if (ehdr.e_type != ET_EXEC && ehdr.e_type != ET_DYN) {
		printf("init_elf(): file '%s' is not a executable file or shared object.\n", elf_file);
		return;
	}
		
	/* 2. get section header table */
  MUXDEF(CONFIG_RV64, Elf64_Shdr, Elf32_Shdr) shdr[ehdr.e_shnum][ehdr.e_shentsize];
		// seek section header table and read 
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
	
	/* 3. get symbol table (symtab) */
		// get symtab table index in section header table
	int symtab_idx = 0;
	for( ; symtab_idx < ehdr.e_shnum; symtab_idx++) {
		if (shdr[symtab_idx]->sh_type == SHT_SYMTAB) {
			break;
		}
	}
	if (symtab_idx == ehdr.e_shnum) {
		printf("init_elf(): Failed to get symtab entry in section header table\n");
		fclose(fp);
		return;
	}

	// fill in static struct symtab 
	symtab.entnum = shdr[symtab_idx]->sh_size / 
											 shdr[symtab_idx]->sh_entsize;
	symtab.entsize = shdr[symtab_idx]->sh_entsize;
	symtab.addr     = shdr[symtab_idx]->sh_offset; 
	
	/*
	if (symtab.entnum > ENTRY_NUM) {
		printf("Please note, ENTRY_NUM should be bigger than %d.\n", symtab.entnum);
		fclose(fp);
		return; 
	}
	*/
	/*
	// see symtab in ELF file and read
	if (fseek(fp, symtab_off, SEEK_SET) != 0) {
		printf("init_elf(): Unable to set symtab postion\n");
		fclose(fp);
		return;
	}

	for( int i = 0; i < symtab.entnum; i++) {
		if (fread(&symtab.sym[i], symtab_entsize, 1, fp) == 0) {
			printf("init_elf(): Unable to get symtab\n");
			fclose(fp);
			return ;
		}
	}
	*/

	/* 4. get string tables (strtab) */
	// 这里我取巧了，直接从 symtab 开始查看，因为 .shstrtab 的 TYPE 也是 STRTAB
		// get strtab table index in section header table
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

	//word_t strtab_off  = shdr[strtab_idx]->sh_offset;
	//word_t strtab_size = shdr[strtab_idx]->sh_size;
	strtab.addr  = shdr[strtab_idx]->sh_offset;
	strtab.size = shdr[strtab_idx]->sh_size;
	
	/*
	if (strtab_size > STRTAB_SIZE) {
		printf("init_elf(): STRTAB_SIZE should bigger than %d.\n", strtab_size);
		fclose(fp);
		return;
	}
		// seek and get strtab
	if ( fseek(fp, strtab_off, SEEK_SET) != 0 ||
			 fread(strtab, strtab_size, 1, fp) == 0 ) {
		printf("init_elf(): Unable to set strtab\n");
		fclose(fp);
		return ;
	}
	*/

	/* printf strtab just for test */
	//print_strtab(strtab, strtab_size);
	//print_strtab2(strtab);

	fclose(fp);
	return;	
} // end function

static void get_symtab_content(char buf[][sizeof(MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym))] ){
	FILE *fp = open_elf();
	assert(fp != NULL);
	// see symtab in ELF file and read
	if (fseek(fp, symtab.addr, SEEK_SET) != 0) {
		printf("Unable to set symtab postion\n");
		fclose(fp);
		return;
	}

	for( int i = 0; i < symtab.entnum; i++) {
		if (fread(&buf[i], symtab.entsize, 1, fp) == 0) {
			printf("Unable to get symtab\n");
			fclose(fp);
			return ;
		}
	}
	fclose(fp);
}

static void get_strtab_content(char* buf) {
	FILE *fp = open_elf();
	assert(fp != NULL);
		// seek and get strtab
	if ( fseek(fp, strtab.addr, SEEK_SET) != 0 ||
			 fread(buf, strtab.size, 1, fp) == 0 ) {
		printf("Unable to get strtab\n");
		fclose(fp);
		return ;
	}
	fclose(fp);
}
/* input addr(pc), &success;
** give a vaddr, if this vaddr of instruction inside a function
** output its function name in strtab and set *boolen = true
** othewise set *boolen = false
** choose == 0, no need to be a function start (the first inst)
** choose == 1, should be the first inst of the function
*/
void vaddr2func(vaddr_t addr, bool *success, int choose, char* func_name, int len){
	*success = false;
	bool cond = false;
	func_name[0] = '\0';
	int i = 0;	
	/* get symtab */
	char symtab_buf[symtab.entnum][symtab.entsize];

	get_symtab_content(symtab_buf);

	MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym) *sym[symtab.entnum];

	for(int idx = 0; idx < symtab.entnum; idx++){
		sym[idx] = (MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym) *)(symtab_buf[idx]);
	}
	
	/* get strtab */
	char strtab_buf[strtab.size];
	memset(strtab_buf, '\0', strtab.size);
	get_strtab_content(strtab_buf);

	for( ; i < symtab.entnum; i++) {
		if ( MUXDEF(CONFIG_RV64, ELF64_ST_TYPE(sym[i]->st_info), 
					ELF32_ST_TYPE(sym[i]->st_info)) == STT_FUNC ) {
	
			if (1 == choose) {
				cond = (addr == sym[i]->st_value);
			} else {
				cond = (addr >= sym[i]->st_value) && 
<<<<<<< HEAD
					(addr <= sym[i]->st_value + sym[i]->st_size);
=======
					(addr < sym[i]->st_value + sym[i]->st_size);
>>>>>>> tracer-ysyx
			}

			if (cond) {
				int k = 0;
				// copy strtab func name to func_name
				for(; strtab_buf[k + sym[i]->st_name] != '\0'; k++){
					if (k >= len){
						printf("vaddr2func(): func name is too long, should increase FUNC_NAME_LEN\n");
						return;
					}
					func_name[k] = strtab_buf[k + sym[i]->st_name ]; 
				}
				func_name[k] = '\0';

				*success = true;
<<<<<<< HEAD
				break;
=======
				return;
				//break;
>>>>>>> tracer-ysyx
			}
		}
	}//end-for
}
