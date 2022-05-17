#include "sched.h"
#include "fork.h"

int nr_tasks = 0;
struct task_struct* task[64] = {0,};

int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg)
{
	struct task_struct *p = (struct task_struct *)get_free_page();
	if (!p)
		return -1;
	
	p->state = TASK_RUNNING;
	p->counter = 0;
	p->preempt_count = 0;
	p->flag          = 0;
	p->cpu_context.x19 = fn;
	p->cpu_context.x20 = arg;
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;
	p->pid            = nr_tasks;
	task[nr_tasks++]  = p;
	task[nr_tasks-1]->next = task[0];
	if(nr_tasks >=2)
		task[nr_tasks-2]->next = task[nr_tasks-1];
	return 0;
}
