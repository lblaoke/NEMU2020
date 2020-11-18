#include "cpu/exec/template-start.h"

#define instr lidt

static void do_execute () {
	switch(op_src->size) {
		case 2:
			cpu.idtr.seg_limit = lnaddr_read(op_src->addr,2);
			cpu.idtr.base_addr = lnaddr_read(op_src->addr+2,3);
			break;
		default:
			cpu.idtr.seg_limit = lnaddr_read(op_src->addr,2);
			cpu.idtr.base_addr = lnaddr_read(op_src->addr+2,4);
	}

	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
