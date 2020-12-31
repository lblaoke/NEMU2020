#include "cpu/exec/template-start.h"

#define make_cmovcc_helper(cc) \
	make_helper(concat4(cmov, cc, _, SUFFIX)) { \
		int len = concat(decode_rm2r_, SUFFIX)(eip + 1); \
		(concat(check_cc_, cc)() ? OPERAND_W(op_dest, op_src->val) : 0 ); \
		print_asm(str(concat(cmov, cc)) str(SUFFIX) " %s,%s", op_src->str, op_dest->str); \
		return len + 1; \
	}

make_cmovcc_helper(e)
make_cmovcc_helper(le)

#include "cpu/exec/template-end.h"
