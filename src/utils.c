#include "utils.h"

void memcpy(void *dst, void *src,size_t num)
{
	int nchunks = num/sizeof(dst);   /*按CPU位宽拷贝*/
	int slice =   num%sizeof(dst);   /*剩余的按字节拷贝*/
	
	unsigned long * s = (unsigned long *)src;
	unsigned long * d = (unsigned long *)dst;
	
	while(nchunks--)
	    *d++ = *s++;
	    
	while (slice--)
	    *((char *)d++) =*((char *)s++);
}