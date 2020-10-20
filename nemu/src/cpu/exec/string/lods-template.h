#include "cpu/exec/template-start.h"

#define instr lods

static void do_execute() {
	switch(DATA_BYTE) {
		case 2:
			reg_w(R_AX)=swaddr_read(reg_w(R_SI),DATA_BYTE,R_DS);
			if(cpu.DF) reg_w(R_SI)-=DATA_BYTE;
			else reg_w(R_SI)+=DATA_BYTE;
			break;
		default:
			REG(R_EAX)=swaddr_read(reg_l(R_ESI),DATA_BYTE,R_DS);
			if(cpu.DF) reg_l(R_ESI)-=DATA_BYTE;
			else reg_l(R_ESI)+=DATA_BYTE;
	}

	print_asm("lods");
}

make_instr_helper(n)

#include "cpu/exec/template-end.h"