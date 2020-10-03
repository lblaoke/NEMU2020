#include <stdio.h>
#include <string.h>
#include "trap.h"

char buf[128];
char str[128]="mytest";

int a[8]={0,1,0,1,0,1,0,1};
int b[8]={0,0,1,1,0,0,1,1};
int c[8]={0,0,0,0,1,1,1,1};

int f(int x) {
	if(x<=0) return x;
	return 1+f(x-1);
}

int main() {
	int i;

	for(i=0;i<8;i++) {
		nemu_assert(!(a[i] && b[i])==(!a[i] || !b[i]));
		nemu_assert(!(a[i] || b[i])==(!a[i] && !b[i]));
		nemu_assert(!!a[i]==a[i]);
		nemu_assert((a[i] && (b[i] || c[i]))==((a[i] && b[i]) || (a[i] && c[i])));
	}

	for(i=0;i<8;i++) {
		nemu_assert(a[i]*(b[i]+c[i])==a[i]*b[i]+a[i]*c[i]);
		nemu_assert(a[i]-b[i]==-(b[i]-a[i]));
		nemu_assert(a[i]+a[i]==a[i]*2);
		nemu_assert(a[i]/2==(a[i]>>1));
		nemu_assert(a[i]-a[i]==0);
	}

	for(i=0;i<8;i++) nemu_assert(i==f(i));

	memset(buf,0xf,sizeof(buf));
	buf[127]=0;
	nemu_assert(strlen(buf)==127);

	strcpy(buf,str);
	nemu_assert(!strcmp(str,"mytest"));

	return 0;
}