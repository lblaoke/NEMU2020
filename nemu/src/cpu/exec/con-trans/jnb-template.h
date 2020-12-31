#include "cpu/exec/template-start.h"

#define instr jnb

static void do_execute() {
	DATA_TYPE_S d = op_src->val;
	if(!cpu.CF) cpu.eip += d;
	
	print_asm_template1();	
}

make_instr_helper(i);

#include "cpu/exec/template-end.h"
