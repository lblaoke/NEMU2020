#include "cpu/exec/template-start.h"

#define instr shl

static void do_execute () {
	DATA_TYPE_S result=op_dest->val;
	int len=(DATA_BYTE<<3)-1;
	uint8_t count=0x1f & op_src->val;

	cpu.CF=!!(result & (1<<(len+1-count)));

	result<<=count;

	OPERAND_W(op_dest,result);

	cpu.OF=0;
	cpu.SF=result>>len;
    cpu.ZF=!result;

	result^=(result>>4);
	result^=(result>>2);
	result^=(result>>1);
	cpu.PF=!(result & 1);

	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"
