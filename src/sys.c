
#include "printf.h"
void sys_write(char * buf){
	printf(buf);
}

void * const sys_call_table[] = {sys_write};
