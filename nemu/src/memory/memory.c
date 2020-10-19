#include "common.h"
#include "burst.h"
#include "memory/memory.h"

/* Memory accessing interfaces */
uint32_t hwaddr_read(hwaddr_t addr,size_t len) {
	Address A;
	A.address=addr;

	uint32_t block1=cache2_read(A),OFFSET=A.offset,zero=0;

	uint8_t temp[4];
	memset(temp,0,sizeof(temp));

	if(OFFSET+len<NR_DATA) {
		memcpy(temp,cache2[block1].data + OFFSET,len);
	} else {
		A.address+=(NR_DATA-OFFSET);
		uint32_t block2=cache2_read(A);

		memcpy(temp,cache2[block1].data + OFFSET, NR_DATA - OFFSET);
		memcpy(temp + NR_DATA - OFFSET,cache2[block2].data, len - (NR_DATA - OFFSET));
	}

	return unalign_rw(temp+zero, 4) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t buf) {
	Address A;
	A.address=addr;
	cache2_write(A,len,buf);
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

