#include "common.h"
#include "burst.h"
#include <stdlib.h>

#define DATA_WIDTH 6
#define GROUP_WIDTH1 7
#define IN_WIDTH1 3

#define NR_DATA (1 << DATA_WIDTH)
#define NR_GROUP1 (1 << GROUP_WIDTH1)
#define NR_IN1 (1 << IN_WIDTH1)

void ddr3_read(hwaddr_t, void *);
uint32_t dram_read(hwaddr_t, size_t);
void ddr3_write(hwaddr_t, void *, uint8_t *);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */
struct Cache1 {
	bool valid;
	uint32_t tag;
	uint8_t data[NR_DATA];
} cache1[NR_GROUP1*NR_IN1];

uint32_t TAG,GROUP,OFFSET;

void init_cache() {
	uint32_t block;
	for(block=0;block<NR_GROUP1*NR_IN1;block++) cache1[block].valid=false;
}

uint32_t cache1_read(hwaddr_t addr) {
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
	bool success=false;
	uint32_t block;
	for(block=GROUP*NR_IN1;block<(GROUP+1)*NR_IN1;block++) if(cache1[block].valid && cache1[block].tag==TAG) {
		success=true;
		break;
	}

	dram_write(addr,len,buf);
	if(success) memcpy(cache1[block].data+OFFSET,&buf,len);
}

uint32_t hwaddr_read(hwaddr_t addr,size_t len) {
	TAG=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	GROUP=(addr>>DATA_WIDTH) & (NR_GROUP1-1);
	OFFSET=addr & (NR_DATA-1);

	uint32_t block1=cache1_read(addr);

	uint8_t temp[4];
	memset(temp,0,sizeof(temp));

	if(OFFSET + len >= NR_DATA) {
		memcpy(temp,cache1[block1].data + OFFSET, NR_DATA - OFFSET);

		GROUP=((addr+len)>>DATA_WIDTH) & (NR_GROUP1-1);
		uint32_t block2=cache1_read(addr+len);

		memcpy(temp + NR_DATA - OFFSET,cache1[block2].data, len - (NR_DATA - OFFSET));
	} else memcpy(temp,cache1[block1].data + OFFSET,len);

	uint32_t zero=0;
	return unalign_rw(temp+zero, 4) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t buf) {
	TAG=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	GROUP=(addr>>DATA_WIDTH) & (NR_GROUP1-1);
	OFFSET=addr & (NR_DATA-1);

	cache1_write(addr,len,buf);
}


uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

