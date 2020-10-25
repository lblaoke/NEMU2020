#include "memory/memory.h"
#include "common.h"
#include <stdlib.h>

void init_tlb() {
	int block;
	for(block=0;block<NR_TLB;block++) tlb[block].valid=false;
	tlb_index=0;
}


uint32_t tlb_read(uint32_t TAG) {
	int block;
	for(block=0;block<tlb_index;block++) if(tlb[block].tag==TAG) return tlb[block].page;
	return -1;
}
void tlb_write(uint32_t TAG,uint32_t PAGE) {
	int block=tlb_index;
	if(block>=NR_TLB) block=rand()%NR_TLB;
	else tlb_index++;

	tlb[block].valid=true;
	tlb[block].tag=TAG;
	tlb[block].page=PAGE;
}