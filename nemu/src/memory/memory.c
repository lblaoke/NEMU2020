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
	uint8_t data[NR_DATA];
	int32_t tag;
	bool valid;
} cache1[NR_GROUP1][NR_IN1];

void init_cache() {
	uint32_t i, j;
	for(i = 0; i < NR_GROUP1; i ++) {
		for(j = 0; j < NR_IN1; j ++) {
			cache1[i][j].valid = false;
		}
	}
}

uint32_t cache1_read(hwaddr_t addr) {
	srand(0);
	uint32_t addr_tag=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	uint32_t group=(addr>>DATA_WIDTH) & (NR_GROUP1-1);

	bool success=false;
	uint32_t in,i;
	for(in=0;in<NR_IN1;in++) if(cache1[group][in].valid && cache1[group][in].tag==addr_tag) {
		success=true;
		break;
	}

	if(!success) {
		for(in=0;in<NR_IN1 && cache1[group][in].valid;in++);
		
		if(in>=NR_IN1) in=rand()%NR_IN1;

		addr&=0xffffffc0;
		for(i=0;i<BURST_LEN;i++) ddr3_read(addr+i*BURST_LEN,cache1[group][in].data+i*BURST_LEN);
		cache1[group][in].tag=addr_tag;
		cache1[group][in].valid=true;
	}

	return in;
}
void cache1_write(hwaddr_t addr,size_t len,uint32_t buf) {
	uint32_t addr_tag=(addr>>(GROUP_WIDTH1+DATA_WIDTH));
	uint32_t group=(addr>>DATA_WIDTH) & (NR_GROUP1-1);
	uint32_t addr_data=addr & (NR_DATA-1);

	bool success=false;
	uint32_t in;
	for(in=0;in<NR_IN1;in++) if(cache1[group][in].valid && cache1[group][in].tag==addr_tag) {
		success=true;
		break;
	}

	dram_write(addr,len,buf);
	if(success) memcpy(cache1[group][in].data+addr_data,&buf,len);
}

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	uint32_t group=(addr>>DATA_WIDTH) & (NR_GROUP1-1);
	uint32_t offset=addr & (NR_DATA-1);
	uint32_t block=cache1_read(addr);
	uint8_t temp[4];
	memset(temp,0,sizeof(temp));

	if(offset + len >= NR_DATA) {
		uint32_t _block = cache1_read(addr + len);
		memcpy(temp,cache1[group][block].data + offset, NR_DATA - offset);
		memcpy(temp + NR_DATA - offset,cache1[group][_block].data, len - (NR_DATA - offset));
	} else memcpy(temp,cache1[group][block].data + offset,len);

	uint32_t zero=0;
	return unalign_rw(temp+zero, 4) & (~0u >> ((4 - len) << 3)); 
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t buf) {
	cache1_write(addr, len, buf);
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

