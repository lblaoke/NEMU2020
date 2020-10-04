#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	long long result=(long long)a*(long long)b;
	return (FLOAT)(result>>16);
}

typedef struct {
	union {
		long long x;
		struct {int l,h;};
	};
} L_t;

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */
	int sign=1;
	if(a>>31) {
		sign=-sign;
		a=-a;
	}
	if(b>>31) {
		sign=-sign;
		b=-b;
	}

	long long A=(long long)a;
	L_t L;
	L.x=(A<<16);

	asm volatile ("div %2" : "=a"(L.l), "=d"(L.h) : "r"(b), "a"(L.l), "d"(L.h));

	return F_mul_int(L.l,sign);

}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */
	int b=*(int *)&a;
	int E=(b>>23) & 0xff;
	int M=b & 0x7fffff;

	if(E) M|=0x800000;
	E-=(127+23-16);

	if(E<0) M>>=(-E);
	else M<<=E;

	return (b>>31)?(-M):M;
}

FLOAT Fabs(FLOAT a) {
	return (a<0)?(-a):a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
		break;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

