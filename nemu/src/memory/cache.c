#include "memory/memory.h"
#include "burst.h"
#include "common.h"
#include <stdlib.h>

void init_cache() {
	uint32_t block;
	for(block=0;block<NR_GROUP1*NR_IN1;block++) cache1[block].valid=false;
}

uint32_t cache1_read(hwaddr_t addr) {
	uint32_t TAG=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	uint32_t GROUP=(addr>>DATA_WIDTH) & (NR_GROUP1-1);

	srand(0);
	bool success=false;
	uint32_t block,i;
	for(block=GROUP*NR_IN1;block<(GROUP+1)*NR_IN1;block++) if(cache1[block].valid && cache1[block].tag==TAG) {
		success=true;
		break;
	}

	if(!success) {
		for(block=GROUP*NR_IN1;block<(GROUP+1)*NR_IN1 && cache1[block].valid;block++);
		
		if(block>=(GROUP+1)*NR_IN1) block=GROUP*NR_IN1+rand()%NR_IN1;

		uint32_t start=((addr>>DATA_WIDTH)<<DATA_WIDTH);
		for(i=0;i<NR_DATA;i+=BURST_LEN) ddr3_read(start+i,cache1[block].data+i);
		cache1[block].tag=TAG;
		cache1[block].valid=true;
	}

	return block;
}

void cache1_write(hwaddr_t addr,size_t len,uint32_t buf) {
	uint32_t TAG=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	uint32_t GROUP=(addr>>DATA_WIDTH) & (NR_GROUP1-1);
	uint32_t OFFSET=addr & (NR_DATA-1);

	bool success=false;
	uint32_t block;
	for(block=GROUP*NR_IN1;block<(GROUP+1)*NR_IN1;block++) if(cache1[block].valid && cache1[block].tag==TAG) {
		success=true;
		break;
	}

	dram_write(addr,len,buf);
	if(success) memcpy(cache1[block].data+OFFSET,&buf,len);
}