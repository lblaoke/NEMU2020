#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute () {
	if(DATA_BYTE==1) {
		swaddr_write(op_src->addr,4,MEM_R(reg_l(R_ESP)));
		MEM_W(reg_l(R_ESP),0);
		reg_l(R_ESP)+=4;
	} else {
		OPERAND_W(op_src,MEM_R(REG(R_ESP)));
		MEM_W(reg_l(R_ESP),0);
		REG(R_ESP)+=DATA_BYTE;
	}

	print_asm_no_template1();
}

#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"