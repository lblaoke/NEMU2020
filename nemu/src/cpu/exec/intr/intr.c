#include <setjmp.h>
#include "nemu.h"

extern jmp_buf jbuf;

void push(int val) {
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP),4,val,R_SS);
}

void raise_intr(uint8_t NO) {
	Assert(NO*8<=cpu.idtr.seg_limit,"number exceeded!");

	GateDesc gate;
	lnaddr_t addr = cpu.idtr.base_addr+NO*8;
	gate.first = lnaddr_read(addr,4);
	gate.second = lnaddr_read(addr+4,4);
	
	push(cpu.eflags);
	if(!cpu.cr0.protect_enable) cpu.IF = cpu.TF = 0;

	push(cpu.cs.selector);
	push(cpu.eip);
	cpu.cs.selector = gate.segment;

	Assert(((cpu.cs.selector>>3)<<3) <= cpu.gdtr.seg_limit,"number exceeded!");

	seg_do(R_CS);
	cpu.eip = cpu.cs.base_addr+gate.offset_15_0+(gate.offset_31_16<<16);

	longjmp(jbuf,1);
}
