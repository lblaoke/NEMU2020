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
	int block;
	for(block=0;block<NR_TLB && tlb[block].valid;block++);

	if(block>=NR_TLB) block=rand()%NR_TLB;

	tlb[block].valid=true;
	tlb[block].tag=TAG;
	tlb[block].page=PAGE;
}