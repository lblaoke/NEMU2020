#include "common.h"
#include "burst.h"
#include "memory/memory.h"
#include "nemu.h"

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
	if(cpu.cr0.protect_enable) {
		Assert(addr+len < cpu.sr[sreg].seg_limit, "CS segment beyond limit!");
		return cpu.sr[sreg].base_addr + addr;
	}
	return addr;	
}

/* Memory accessing interfaces */
uint32_t hwaddr_read(hwaddr_t addr,size_t len) {
	Address A;
	A.address=addr;

	uint32_t block1=cache1_read(A),OFFSET=A.offset,zero=0;

	uint8_t temp[4];
	memset(temp,0,sizeof(temp));

	if(OFFSET+len<=NR_DATA) {
		memcpy(temp,cache1[block1].data + OFFSET,len);
	} else {
		A.address+=(NR_DATA-OFFSET);
		uint32_t block2=cache1_read(A);

		memcpy(temp,cache1[block1].data + OFFSET, NR_DATA - OFFSET);
		memcpy(temp + NR_DATA - OFFSET,cache1[block2].data, len - (NR_DATA - OFFSET));
	}

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

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}


void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_write(lnaddr, len, data);
}

