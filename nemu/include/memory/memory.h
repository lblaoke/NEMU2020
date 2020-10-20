#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

#define HW_MEM_SIZE (128 * 1024 * 1024)

#define DATA_WIDTH 6
#define GROUP_WIDTH1 7
#define GROUP_WIDTH2 12
#define IN_WIDTH1 3
#define IN_WIDTH2 0

#define NR_DATA 64
#define NR_GROUP1 128
#define NR_GROUP2 4096
#define NR_IN1 8
#define NR_IN2 1

extern uint8_t *hw_mem;

/* convert the hardware address in the test program to virtual address in NEMU */
#define hwa_to_va(p) ((void *)(hw_mem + (unsigned)p))
/* convert the virtual address in NEMU to hardware address in the test program */
#define va_to_hwa(p) ((hwaddr_t)((void *)p - (void *)hw_mem))

#define hw_rw(addr, type) *(type *)({\
	Assert(addr < HW_MEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
	hwa_to_va(addr); \
})

typedef union {
	uint32_t address;
	struct {
		uint32_t offset	:DATA_WIDTH;
		uint32_t group1	:GROUP_WIDTH1;
		uint32_t tag1	:(32-GROUP_WIDTH1-DATA_WIDTH);
	};
	struct {
		uint32_t offset	:DATA_WIDTH;
		uint32_t group2	:GROUP_WIDTH2;
		uint32_t tag2	:(32-GROUP_WIDTH2-DATA_WIDTH);
	};
} Address;

typedef struct {
	bool valid;
	uint32_t tag;
	uint8_t data[NR_DATA];	
}Cache1;
typedef struct {
	bool valid,dirty;
	uint32_t tag;
	uint8_t data[NR_DATA];	
}Cache2;

Cache1 cache1[NR_GROUP1*NR_IN1];
Cache2 cache2[NR_GROUP2*NR_IN2];

void cache_init();
uint32_t cache1_read(Address);
uint32_t cache2_read(Address);
void cache1_write(Address,size_t,uint32_t);
void cache2_write(Address,size_t,uint32_t);

void ddr3_read(hwaddr_t, void *);
uint32_t dram_read(hwaddr_t, size_t);
void ddr3_write(hwaddr_t, void *, uint8_t *);
void dram_write(hwaddr_t, size_t, uint32_t);

uint32_t swaddr_read(swaddr_t, size_t);
uint32_t lnaddr_read(lnaddr_t, size_t);
uint32_t hwaddr_read(hwaddr_t, size_t);
void swaddr_write(swaddr_t, size_t, uint32_t);
void lnaddr_write(lnaddr_t, size_t, uint32_t);
void hwaddr_write(hwaddr_t, size_t, uint32_t);

#endif
