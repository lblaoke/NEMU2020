#include "cpu/exec/template-start.h"

#define instr lods

static void do_execute() {
	switch(DATA_BYTE) {
		case 2:
			reg_w(R_AL)=swaddr_read(reg_w(R_SI),DATA_BYTE);
			if(cpu.DF) reg_w(R_SI)-=DATA_BYTE;
			else reg_w(R_SI)+=DATA_BYTE;
			break;
		default:
			REG(R_EAX)=swaddr_read(reg_l(R_ESI),DATA_BYTE);
			if(cpu.DF) REG(R_ESI)-=DATA_BYTE;
			else REG(R_ESI)+=DATA_BYTE;
	}

	print_asm("lods");
}

make_instr_helper(n)

#include "cpu/exec/template-end.h"