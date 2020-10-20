#include "cpu/exec/template-start.h"

#define instr lgdt

static void do_execute () {
	cpu.gdtr.seg_limit = lnaddr_read (op_src->addr , 2);
	switch(op_src->size) {
		case 2:
			cpu.gdtr.base_addr = lnaddr_read (op_src->addr + 2,3);
			break;
		case 4:
			cpu.gdtr.base_addr = lnaddr_read (op_src->addr + 2,4);
	}
	
	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
