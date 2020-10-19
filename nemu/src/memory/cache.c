#include "memory/memory.h"
#include "burst.h"
#include "common.h"
#include <stdlib.h>

void init_cache() {
	uint32_t block;
	for(block=0;block<NR_GROUP1*NR_IN1;block++) cache1[block].valid=false;
}

uint32_t cache1_read(Address addr) {
	uint32_t start=addr.group1*NR_IN1,end=(addr.group1+1)*NR_IN1;

	uint32_t block;
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
	bool success=false;
	uint32_t block;
	for(block=addr.group1*NR_IN1;block<(addr.group1+1)*NR_IN1;block++) if(cache1[block].valid && cache1[block].tag==addr.tag1) {
		success=true;
		break;
	}

	dram_write(addr.address,len,buf);
	if(success) memcpy(cache1[block].data+addr.offset,&buf,len);
}