#include "memory/memory.h"
#include "common.h"
#include <stdlib.h>

void init_tlb() {
	int block;
	for(block=0;block<NR_TLB;block++) tlb[block].valid=false;
	tlb_index=0;
	tlb_rand_index=-1;
}


uint32_t tlb_read(uint32_t TAG) {
	int block;
	if(tlb_rand_index>-1 && tlb[tlb_rand_index].tag==TAG) return tlb[tlb_rand_index].page;
	for(block=0;block<tlb_index;block++) if(tlb[block].tag==TAG) return tlb[block].page;
	return -1;
}
void tlb_write(uint32_t TAG,uint32_t PAGE) {
	int block=tlb_index;
	if(block>=NR_TLB) {
		tlb_rand_index=rand()%NR_TLB;
		block=tlb_rand_index;
	} else {
		tlb_rand_index=tlb_index;
		tlb_index++;
	}

	tlb[block].valid=true;
	tlb[block].tag=TAG;
	tlb[block].page=PAGE;
}