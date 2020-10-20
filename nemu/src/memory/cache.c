#include "memory/memory.h"
#include "burst.h"
#include "common.h"
#include <stdlib.h>

void init_cache() {
	uint32_t block;
	for(block=0;block<NR_GROUP1*NR_IN1;block++) cache1[block].valid=false;
	for(block=0;block<NR_GROUP2*NR_IN2;block++) cache2[block].valid=false;
}

uint32_t cache1_read(Address addr) {
	uint32_t block,start=addr.group1*NR_IN1,end=(addr.group1+1)*NR_IN1;

	for(block=start;block<end;block++) if(cache1[block].valid && cache1[block].tag==addr.tag1) return block;

	//find free cache
	for(block=start;block<end && cache1[block].valid;block++);
	if(block>=end) {
		srand(block);
		block=start+rand()%NR_IN1;
	}

	uint32_t block2=cache2_read(addr);
	cache1[block].valid=true;
	cache1[block].tag=addr.tag1;
	memcpy(cache1[block].data,cache2[block2].data,NR_DATA);

	return block;
}
uint32_t cache2_read(Address addr) {
	uint32_t i,block,start=addr.group2*NR_IN2,end=(addr.group2+1)*NR_IN2;

	for(block=start;block<end;block++) if(cache2[block].valid && cache2[block].tag==addr.tag2) return block;

	//find free cache
	//for(block=start;block<end && cache2[block].valid;block++);

	//if(block>=end) {
	{
		srand(block);
		block=start+rand()%NR_IN2;
		if(cache2[block].valid && cache2[block].dirty) {
			printf("write back!\n");
			uint8_t mask[BURST_LEN<<1];
			memset(mask,1,BURST_LEN<<1);

			Address B;
			B.address=0;
			B.tag2=cache2[block].tag;
			B.group2=block;

			for(i=0;i<NR_DATA;i+=BURST_LEN) ddr3_write(B.address+i,cache2[block].data+i,mask);
			printf("success!\n");
		}
	}

	addr.address-=addr.offset;

	cache2[block].valid=true;
	cache2[block].dirty=false;
	cache2[block].tag=addr.tag2;
	for(i=0;i<NR_DATA;i+=BURST_LEN) ddr3_read(addr.address+i,cache2[block].data+i);

	return block;
}

void cache1_write(Address addr,size_t len,uint32_t buf) {
	uint32_t block,start=addr.group1*NR_IN1,end=(addr.group1+1)*NR_IN1;

	cache2_write(addr,len,buf);

	for(block=start;block<end;block++) if(cache1[block].valid && cache1[block].tag==addr.tag1) {
		uint32_t block2=cache2_read(addr);
		memcpy(cache1[block].data,cache2[block2].data,NR_DATA);
		break;
	}
}
void cache2_write(Address addr,size_t len,uint32_t buf) {
	uint32_t block,start=addr.group2*NR_IN2,end=(addr.group2+1)*NR_IN2,OFFSET=addr.offset;

	for(block=start;block<end;block++) if(cache2[block].valid && cache2[block].tag==addr.tag2) break;
	if(block>=end) block=cache2_read(addr);

	cache2[block].dirty=true;

	if(OFFSET+len<=NR_DATA) memcpy(cache2[block].data+OFFSET,&buf,len);
	else {
		uint8_t temp[4];
		memcpy(temp,&buf,4);

		memcpy(cache2[block].data+OFFSET,temp,NR_DATA-OFFSET);

		addr.address+=(NR_DATA-OFFSET);
		block=cache2_read(addr);

		memcpy(cache2[block].data,temp+NR_DATA-OFFSET,len-(NR_DATA-OFFSET));
	}

}