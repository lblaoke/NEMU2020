#include "cpu/exec/helper.h"

static inline bool check_cc_b() {
	return cpu.CF;
}

static inline bool check_cc_e() {
	return cpu.ZF;
}

static inline bool check_cc_ne() {
	return !cpu.ZF;
}

static inline bool check_cc_be() {
	return cpu.CF | cpu.ZF;
}

static inline bool check_cc_a() {
	return !(cpu.CF | cpu.ZF);
}

static inline bool check_cc_s() {
	return cpu.SF;
}

static inline bool check_cc_ns() {
	return !cpu.SF;
}


static inline bool check_cc_l() {
	return cpu.SF ^ cpu.OF;
}

static inline bool check_cc_ge() {
	return !(cpu.SF ^ cpu.OF);
}

static inline bool check_cc_le() {
	return (cpu.SF ^ cpu.OF) | cpu.ZF;
}

static inline bool check_cc_g() {
	return !((cpu.SF ^ cpu.OF) | cpu.ZF);
}

#define DATA_BYTE 2
#include "cmove-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmove-template.h"
#undef DATA_BYTE

make_helper_v(cmove)
make_helper_v(cmovle)
