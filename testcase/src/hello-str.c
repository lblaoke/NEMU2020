#include <stdio.h>
#include <string.h>
#include "trap.h"

char buf[128];

int main() {
	sprintf(buf, "%s", "a");
	nemu_assert(!strcmp(buf, "a"));
/*
	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	nemu_assert(strcmp(buf, "1 + 1 = 2\n") == 0);

	sprintf(buf, "%d + %d = %d\n", 2, 10, 12);
	nemu_assert(strcmp(buf, "2 + 10 = 12\n") == 0);
*/
	return 0;
}
