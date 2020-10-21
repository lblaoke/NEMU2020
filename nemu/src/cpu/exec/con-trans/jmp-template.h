#include "cpu/exec/template-start.h"
#include "../../../../../lib-common/x86-inc/mmu.h"

#define instr jmp

static void do_execute() {
	DATA_TYPE_S displacement=op_src->val;
	
	if (op_src->type==OP_TYPE_REG || op_src->type==OP_TYPE_MEM) {
		cpu.eip=displacement-concat(decode_rm_,SUFFIX)(cpu.eip+1)-1;

		print_asm_no_template1();
	} else {
		cpu.eip+=displacement;

		print_asm("jmp %x",cpu.eip+1+DATA_BYTE);
	}
}

#if DATA_BYTE == 4
make_helper(ljmp) {
	extern SegDesc *seg_des;
	SegDesc seg;
	seg_des=&seg;

	uint32_t op1 = instr_fetch(eip+1, 4);
	uint16_t op2 = instr_fetch(eip +5, 2);
	cpu.eip = op1;
	cpu.cs.selector = op2;

	Assert(((cpu.cs.selector>>3)<<3) <= cpu.gdtr.seg_limit, "segment out of limit %d, %d", ((cpu.cs.selector>>3)<<3), cpu.gdtr.seg_limit);
	seg_des->_0 = instr_fetch(cpu.gdtr.base_addr + ((cpu.cs.selector>>3)<<3), 4);
	seg_des->_4 = instr_fetch(cpu.gdtr.base_addr + ((cpu.cs.selector>>3)<<3)+4, 4);

	Assert(seg_des->present == 1, "segment error!");
	cpu.cs.seg_base1 = seg_des->base_15_0;
	cpu.cs.seg_base2 = seg_des->base_23_16;
	cpu.cs.seg_base3 = seg_des->base_31_24;
	cpu.cs.seg_limit1 = seg_des->limit_15_0;
	cpu.cs.seg_limit2 = seg_des->limit_19_16;
	cpu.cs.seg_limit3 = 0xfff;

	print_asm("ljmp %x,%x", op2, op1);
	return 0;
}
#endif

make_instr_helper(i)
make_instr_helper(rm)

#include "cpu/exec/template-end.h"