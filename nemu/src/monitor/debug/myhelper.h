#ifndef __MYHELPER_H__
#define __MYHELPER_H__

#include "nemu.h"
#include "common.h"
#include <stdlib.h>
#include <elf.h>

char *strtab = NULL;
Elf32_Sym *symtab = NULL;
int nr_symtab_entry;

typedef struct {
    swaddr_t prev_ebp;
    swaddr_t ret_addr;
    uint32_t args[4];
} PartOfStackFrame;

uint32_t elf_value(char *s) {
	int i;
	for(i=0;i<nr_symtab_entry;i++) if((symtab[i].st_info & 0xf)==STT_OBJECT) {
		char tmp[32];
		strcpy(tmp,strtab+symtab[i].st_name);
		if(!strcmp(tmp,s)) return symtab[i].st_value;
	}
	return 0;
}

void read_ebp (swaddr_t addr , PartOfStackFrame *ebp) {
	ebp -> prev_ebp = swaddr_read (addr , 4);
	ebp -> ret_addr = swaddr_read (addr + 4 , 4);
	int i;
	for (i = 0;i < 4;i ++) ebp -> args [i] = swaddr_read (addr + 8 + 4 * i , 4);
}

#endif