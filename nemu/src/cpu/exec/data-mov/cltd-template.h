#include "cpu/exec/template-start.h"

#define instr cltd

static void do_execute () {
	DATA_TYPE len=(DATA_BYTE<<1)-1;

	REG(R_EDX)=0;
	if(REG(R_EAX)>>len) REG(R_EDX)--;

	print_asm("cltd");
}

make_instr_helper(n)

#include "cpu/exec/template-end.h"
