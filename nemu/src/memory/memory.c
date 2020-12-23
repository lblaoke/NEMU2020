#include "common.h"
#include "burst.h"
#include "memory/memory.h"
#include "nemu.h"

int is_mmio(hwaddr_t);
uint32_t mmio_read(hwaddr_t,size_t,int);
void mmio_write(hwaddr_t,size_t,uint32_t,int);

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
	if(cpu.cr0.protect_enable) {
		Assert(addr+len < cpu.sr[sreg].seg_limit, "CS segment beyond limit!");
		return cpu.sr[sreg].base_addr + addr;
	}
	return addr;
}

hwaddr_t page_translate(lnaddr_t addr) {
	if(cpu.cr0.paging && cpu.cr0.protect_enable) {
		//printf("0x%08x\n",addr);
		//Assert(0,"0x%08x",addr);
		Address A,B;
		A.address=addr;

		// B.address=tlb_read(A.tag);
		// if(B.address!=-1) {
		// 	A.tag=B.address;
		// 	return A.address;
		// }

		PTE dir_1,page_1;

		B.tag=cpu.cr3.page_directory_base;
		B.OFFSET=(A.DIR<<2);
		dir_1.val = hwaddr_read(B.address,4);

		Assert(dir_1.present, "page value = %x, eip = %x", dir_1.val,cpu.eip);
		B.tag=dir_1.page_frame;
		B.OFFSET=(A.PAGE<<2);
		page_1.val = hwaddr_read(B.address,4);

		Assert(page_1.present, "page do not exist at %x", cpu.eip);
		A.tag=page_1.page_frame;

		//tlb_write(A.tag,page_1.page_frame);
		return A.address;
	}

	return addr;
}



/* Memory accessing interfaces */
uint32_t hwaddr_read(hwaddr_t addr,size_t len) {
	int map_id = is_mmio(addr);
	if(map_id!=-1) return mmio_read(addr,len,map_id) & (~0u>>((4-len)<<3));	

	Address A;
	A.address = addr;

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
	int map_id = is_mmio(addr);
	if(map_id!=-1) {
		mmio_write(addr,len,buf,map_id);
		return;
	}

	Address A;
	A.address=addr;
	cache1_write(A,len,buf);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
/*
	size_t max_len = ((~addr) & 0xfff) + 1;
    if (len > max_len) {
    	uint32_t low = lnaddr_read(addr, max_len);
        uint32_t high = lnaddr_read(addr + max_len, len - max_len);
        return (high << (max_len << 3)) | low;
    }
*/
	hwaddr_t hwaddr = page_translate(addr);
	return hwaddr_read(hwaddr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
/*
	size_t max_len = ((~addr) & 0xfff) + 1;
    if (len > max_len) {
    	lnaddr_write(addr, max_len, data & ((1 << (max_len << 3)) - 1));
        lnaddr_write(addr + max_len, len - max_len, data >> (max_len << 3));
        return;
    }
*/
	hwaddr_t hwaddr = page_translate(addr);
	//printf("0x%08x -> 0x%08x\n",addr,hwaddr);
	//Assert(addr==hwaddr,"lala");
	hwaddr_write(hwaddr, len, data);
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

