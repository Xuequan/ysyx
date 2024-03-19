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
#include <string.h>

extern char *elf_file;

#define ENTRY_NUM 100
static struct {
	// actual symtab entry number
	word_t entnum;
	MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym) sym[ENTRY_NUM];
} symtab;

#define STRTAB_SIZE 1024
static char strtab[STRTAB_SIZE];	


/* get symtab from reading ELF file 
** FILE *elf_fp is the opening fd of ELF file 
** char *buf[][] is &symtab[symtab.entnum][symtab.entsize]
** return true if success, otherwise false.
*/
/*
bool get_symtab(FILE *elf_fp,  buf[][symtab.entsize]) {

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
		if (fread(buf[i], symtab.entsize, 1, elf_fp) == 0) {
			printf("get_symtab(): Unable to get symtab\n");
			fclose(elf_fp);
			return false;
		}
	}
	return true;
}
*/

// 测试，打印 strtab
void print_strtab(char buf[], int strtab_size) {

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
void print_strtab2(char buf[]) {
	printf("========== print2 strtab start =============\n");
	int i = 0;	
	char *ret;
	for( ; i < symtab.entnum; i++) {
		ret = buf + symtab.sym[i].st_name; 	
		printf("%d - %s\n", symtab.sym[i].st_name, ret);
	}//end-for
	printf("========== print2 strtab end =============\n");
}
/* get strtab from reading ELF file 
** FILE *elf_fp is the opening fd of ELF file 
** char *buf is the buf[strtab.size]
** return true if success, otherwise false.
*/
/*
bool get_strtab(FILE *elf_fp, char* buf) {

	if (elf_fp == NULL) {
		FILE *elf_fp = fopen(elf_file, "r");
		if (elf_fp == NULL) {
			printf("get_strtab(): Can not open '%s'\n", elf_file);
			return false;
		}
	}
	// 定位到 strtab 的位置
	if ( fseek(elf_fp, strtab.off, SEEK_SET) != 0) {
		printf("get_strtab(): Unable to set strtab postion\n");
		return false;
	}

	if ( fread(buf, strtab.size, 1, elf_fp) == 0) {
		printf("get_strtab(): Cannot get strtab\n");
		return false;
	}	
	return true;
}
*/
/*
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

*/
/* read from ELF file and get strtab & symtab */
void init_elf() {
	FILE *fp = fopen(elf_file, "r");

	if (fp == NULL) {
		printf("init_elf(): Can not open '%s'\n", elf_file);
		return;
	}

	/* 1. get Ehdr and ELF file check */
	MUXDEF(CONFIG_RV64, Elf64_Ehdr, Elf32_Ehdr) ehdr;
	if ( fread(&ehdr, sizeof(ehdr), 1, fp) == 0) {
		printf("init_elf(): Cannot read ElfN_Ehdr.\n");
		fclose(fp);
		return;
	}

	// check elf_file is a ELF file
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
	if (ehdr.e_type != ET_EXEC && ehdr.e_type != ET_DYN) {
		printf("init_elf(): file '%s' is not a executable file or shared object.\n", elf_file);
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
		printf("init_elf(): Failed to get symtab entry in section header table\n");
		fclose(fp);
		return;
	}

	// fill in static struct symtab 
	symtab.entnum = shdr[symtab_idx]->sh_size / 
											 shdr[symtab_idx]->sh_entsize;
	word_t symtab_entsize = shdr[symtab_idx]->sh_entsize;
	word_t symtab_off     = shdr[symtab_idx]->sh_offset; 
	
	if (symtab.entnum > ENTRY_NUM) {
		printf("Please note, ENTRY_NUM should be bigger than %#x.\n", symtab.entnum);
		fclose(fp);
		return; 
	}

	// 定位到 symtab
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

	word_t strtab_off  = shdr[strtab_idx]->sh_offset;
	word_t strtab_size = shdr[strtab_idx]->sh_size;
	
	if (strtab_size > STRTAB_SIZE) {
		printf("init_elf(): STATAB_SIZE should bigger than %#x.\n", strtab_size);
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

	/* printf strtab just for test */
	print_strtab(strtab, strtab_size);
	print_strtab2(strtab);

	fclose(fp);
	return;	
} // end function

/* give a vaddr, if this vaddr of instruction inside a function
** output its function name in strtab and set *boolen = true
** othewise set *boolen = false
*/
/*
char *vaddr2func(vaddr_t addr, bool *success, vaddr_t dnpc){

	*success = false;

	char *ret = NULL;
	int i = 0;	
	for( ; i < symtab.entnum; i++) {
		if (addr >= symtab.sym[i].st_value && 
				addr <= symtab.sym[i].st_value + symtab.sym[i].st_size){
			if ( MUXDEF(CONFIG_RV64, ELF64_ST_TYPE(symtab.sym[i].st_info), ELF32_ST_TYPE(symtab.sym[i].st_info)) == STT_FUNC ) {
				ret = strtab + symtab.sym[i].st_name; 	
				//printf("FUNC name = %s\n", ret);
				*success = true;
				break;
			}
		} 
	}//end-for
	if ( i == symtab.entnum ) {
		*success = false;
		return ret;
	} 
		
	int j = 0;	
	for( ; j != i && j < symtab.entnum; j++ ){
		if (dnpc == symtab.sym[j].st_value){
			if ( MUXDEF(CONFIG_RV64, ELF64_ST_TYPE(symtab.sym[j].st_info), ELF32_ST_TYPE(symtab.sym[j].st_info)) == STT_FUNC ) {
			// now pc at addr will call a function or ret 
			}
		}
	}//end-for

	return ret;	
}
*/
/* input addr(pc), &success;
** choose == 0, no need to be a function start (the first inst)
** choose == 1, should be the first inst of the function
*/
char *vaddr2func(vaddr_t addr, bool *success, int choose){

	*success = false;
	char *ret = NULL;
	bool cond = false;
	int i = 0;	
	for( ; i < symtab.entnum; i++) {
		if (1 == choose) {
			cond = (addr == symtab.sym[i].st_value);
		} else {
			cond = (addr >= symtab.sym[i].st_value && 
				addr <= symtab.sym[i].st_value + symtab.sym[i].st_size);
		}

		if (cond) {
			if ( MUXDEF(CONFIG_RV64, ELF64_ST_TYPE(symtab.sym[i].st_info), ELF32_ST_TYPE(symtab.sym[i].st_info)) == STT_FUNC ) {
				ret = strtab + symtab.sym[i].st_name; 	
				*success = true;
				break;
			}
		} 
	}//end-for

	return ret;	
}
