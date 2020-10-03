#include <stdio.h>
#include <string.h>
#include "trap.h"

char buf[128];

int main() {

	sprintf(buf, "%s", "1");
	nemu_assert(!strcmp(buf, "1"));

	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	nemu_assert(strcmp(buf, "1 + 1 = 2\n") == 0);

	sprintf(buf, "%d", 0);
	nemu_assert(strcmp(buf, "0") == 0);

	return 0;
}
