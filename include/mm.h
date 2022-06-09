#ifndef	_MM_H
#define	_MM_H
#include "peripherals/base.h"
#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_MASK			0xfffffffffffff000
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

//8M
#define LOW_MEMORY    (2 * SECTION_SIZE)
//1G
#define HIGH_MEMORY   PBASE

#define PAGESIZE      (1<<PAGE_SHIFT)

#define THREAD_SIZE    PAGESIZE

#define PAGING_MEMORY 			(HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES 			(PAGING_MEMORY/PAGE_SIZE)

#define PTRS_PER_TABLE			(1 << TABLE_SHIFT)

#define PGD_SHIFT			PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			PAGE_SHIFT + TABLE_SHIFT

#define PG_DIR_SIZE			(3 * PAGE_SIZE)

#define VA_START 			0xffff000000000000
#define PHYS_MEMORY_SIZE 		0x40000000

#ifndef __ASSEMBLER__
#include "sched.h"

void memzero(unsigned long src, unsigned long n);
void memcpy(unsigned long dst, unsigned long src, unsigned long n);
unsigned long get_free_page();
void free_page(unsigned long p);
void * allocate_kernel_page();
void * allocate_user_page(struct task_struct *task, unsigned long va);
int kernel_map(unsigned long va);
//int copy_virt_memory(struct task_struct *dst);
#endif

#endif  /*_MM_H */

