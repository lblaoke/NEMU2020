#include "memory/memory.h"
#include "burst.h"
#include "common.h"
#include <stdlib.h>

void init_cache() {
	uint32_t block;
	for(block=0;block<NR_GROUP1*NR_IN1;block++) cache1[block].valid=false;
}

uint32_t cache1_read(Address addr) {
	uint32_t block,start=addr.group1*NR_IN1,end=(addr.group1+1)*NR_IN1;

	for(block=start;block<end;block++) if(cache1[block].valid && cache1[block].tag==addr.tag1) return block;

	//find free cache
	for(block=start;block<end && cache1[block].valid;block++);
	
	//swap
	if(block>=end) {
		srand(0);
		block=start+rand()%NR_IN1;
	}

	//load new data
	addr.address=((addr.address>>DATA_WIDTH)<<DATA_WIDTH);
	uint32_t i;
	for(i=0;i<NR_DATA;i+=BURST_LEN) ddr3_read(addr.address+i,cache1[block].data+i);
	cache1[block].tag=addr.tag1;
	cache1[block].valid=true;

	return block;
}

void cache1_write(Address addr,size_t len,uint32_t buf) {
	uint32_t block,start=addr.group1*NR_IN1,end=(addr.group1+1)*NR_IN1;

	dram_write(addr.address,len,buf);

	for(block=start;block<end;block++) if(cache1[block].valid && cache1[block].tag==addr.tag1) {
		memcpy(cache1[block].data+addr.offset,&buf,len);
		break;
	}
}