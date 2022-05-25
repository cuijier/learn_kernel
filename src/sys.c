
#include "printf.h"
#include "current.h"
#include "sched.h"
#include "sysregs.h"
void sys_write(char * buf){
	struct pt_regs* pregs = get_task_pt_regs(current);
	pregs->regs[0] = printf(buf);
}

int sys_clone(void * stack)
{
	return do_fork(0,0,0, stack);
}

unsigned long sys_malloc()
{
	unsigned long addr = get_free_page();
	if (!addr) {
		return -1;
	}
	return addr;
}

void sys_exit()
{
	exit_process();
}

void * const sys_call_table[] = {sys_write, sys_malloc, sys_clone, sys_exit};
