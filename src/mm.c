#include "mm.h"

#define PAGE_NUM         (HIGH_MEMORY - LOW_MEMORY)/PAGESIZE
static char mem_map[PAGE_NUM] = {0,};


unsigned long get_free_page()
{
    for(int i = 0 ; i< PAGE_NUM ; i++)
    {
        if(!mem_map[i])
	{
	    mem_map[i] = 1;
	    return LOW_MEMORY + PAGESIZE * i;
	}
    }
    return 0;
}


void free_page(unsigned long p)
{
    mem_map[(p - LOW_MEMORY)/PAGESIZE] = 0;
}
