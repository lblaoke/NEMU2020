#include "memory/memory.h"
#include "common.h"
#include <stdlib.h>

void init_tlb() {
	int block;
	for(block=0;block<NR_TLB;block++) tlb[block].valid=false;
}


uint32_t tlb_read(uint32_t TAG) {
	int block;
	for(block=0;block<NR_TLB;block++) if(tlb[block].valid && tlb[block].tag==TAG) return tlb[block].page;
	return -1;
}
void tlb_write(uint32_t TAG,uint32_t PAGE) {
	int block=0,left=0,right=NR_TLB-1,middle;
	//for(block=0;block<NR_TLB && tlb[block].valid;block++);
	while(left<=right) {
		if(!tlb[left].valid) {
			block=left;
			break;
		}
		if(tlb[right].valid) {
			block=-1;
			break;
		}
		middle=(left+right)>>1;
		if(tlb[middle].valid) left=middle+1;
		else right=middle;
	}

	if(block==-1) block=rand()%NR_TLB;

	tlb[block].valid=true;
	tlb[block].tag=TAG;
	tlb[block].page=PAGE;
}