#include <stdio.h>
#include <string.h>
#include "trap.h"

char buf[128];
char str[128]="mytest";

int a[8]={0,1,0,1,0,1,0,1};
int b[8]={0,0,1,1,0,0,1,1};
int c[8]={0,0,0,0,1,1,1,1};

int main() {
	int i;

	for(i=0;i<8;i++) {
		nemu_assert(!(a[i] && b[i])==(!a[i] || !b[i]));
		nemu_assert(!(a[i] || b[i])==(!a[i] && !b[i]));
		nemu_assert(!!a[i]==a[i]);
		nemu_assert((a[i] && (b[i] || c[i]))==((a[i] && b[i]) || (a[i] && c[i])));
	}
/*
	memset(buf,0,sizeof(buf));
	nemu_assert(strlen(buf)==128);
	strcpy(buf,str);
	nemu_assert(!strcmp(str,"mytest"));
*/
	return 0;
}