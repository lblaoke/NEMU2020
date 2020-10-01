#include "cpu/exec/template-start.h"

#define instr scas

static void do_execute() {
	DATA_TYPE a,dest;

	switch(DATA_BYTE) {
		case 1:
			a=reg_w(R_AL);
			dest=swaddr_read(reg_w(R_EDI),DATA_BYTE);
			break;
		case 2:
			a=reg_w(R_AX);
			dest=swaddr_read(reg_w(R_DI),DATA_BYTE);
			break;
		default:
			a=reg_w(R_EAX);
			dest=swaddr_read(reg_w(R_EDI),DATA_BYTE);
	}
	DATA_TYPE result=a-dest;

	int len=(DATA_BYTE<<3)-1;
	int s1=a>>len,s2=dest>>len;

	cpu.CF=a<dest;
	cpu.SF=result>>len;
	cpu.ZF=!result;
    cpu.OF=(s1!=s2 && s2==cpu.SF);

	result^=(result>>4);
	result^=(result>>2);
	result^=(result>>1);
	cpu.PF=!(result & 1);

	print_asm("scas");
}

make_instr_helper(n)

#include "cpu/exec/template-end.h"