#include "cpu/exec/template-start.h"

#define instr sar

static void do_execute () {
	DATA_TYPE_S result=op_dest->val;
	int len=(DATA_BYTE<<3)-1;

	result>>=(uint8_t)(0x1f & op_src->val);

	OPERAND_W(op_dest,result);

	cpu.CF=cpu.OF=0;
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
