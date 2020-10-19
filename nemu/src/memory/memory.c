#include "common.h"
#include "burst.h"
#include "memory/memory.h"

/* Memory accessing interfaces */
uint32_t hwaddr_read(hwaddr_t addr,size_t len) {
	Address A;
	A.address=addr;

	uint32_t block1=cache1_read(A);

	uint8_t temp[4];
	memset(temp,0,sizeof(temp));

	if(A.offset + len >= NR_DATA) {
		memcpy(temp,cache1[block1].data + A.offset, NR_DATA - A.offset);

		Address B;
		B.address=A.address+len;
		uint32_t block2=cache1_read(B);

		memcpy(temp + NR_DATA - A.offset,cache1[block2].data, len - (NR_DATA - A.offset));
	} else memcpy(temp,cache1[block1].data + A.offset,len);

	uint32_t zero=0;
	return unalign_rw(temp+zero, 4) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t buf) {
	Address A;
	A.address=addr;
	cache1_write(A,len,buf);
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

