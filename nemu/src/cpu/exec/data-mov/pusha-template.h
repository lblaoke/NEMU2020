#include "cpu/exec/template-start.h"

#define instr pusha

make_helper(concat(pusha_1_,SUFFIX)) {
	int i;
	DATA_TYPE tmp;
	switch(DATA_BYTE) {
		case 2:
			tmp = REG(R_SP);
			for(i=R_AX;i<=R_DI;i++) {
				REG(R_SP) -= 2;
				if(i==R_SP) MEM_W(REG(R_SP),tmp,R_SS);	
				else MEM_W(REG(R_SP),REG(i),R_SS);
			}
			break;
		default:
			tmp = REG(R_ESP);
			for(i=R_EAX;i<=R_EDI;i++) {
				REG(R_ESP) -= 4;
				if(i==R_ESP) MEM_W(REG(R_ESP),tmp,R_SS);	
				else MEM_W(REG(R_ESP),REG(i),R_SS);
			}
	}

	return 1;
}

#include "cpu/exec/template-end.h"
