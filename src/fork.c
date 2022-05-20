#include "sched.h"
#include "fork.h"
#include "current.h"
#include "printf.h"

int nr_tasks = 0;
struct task_struct* task[64] = {0,};

int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack)
{
	printf("%s enter\n",__func__);
	preempt_disable();
	struct task_struct *p = (struct task_struct *)get_free_page();
	if (!p)
		return -1;

	struct pt_regs * ptr = get_task_pt_regs(p);
	memzero((unsigned long)ptr, sizeof(struct pt_regs));
	memzero((unsigned long)&p->cpu_context, sizeof(struct cpu_context));

	p->state = TASK_RUNNING;
	p->counter = 0;
	p->preempt_count = 0;
	p->flag          = 0;
	if(PF_KTHREAD & clone_flags)
	{
		p->cpu_context.x19 = fn;
		p->cpu_context.x20 = arg;
	}
	else
	{
		p->stack = stack;
		struct pt_regs * cur_regs = get_task_pt_regs(current);
		//*ptr                = *cur_regs;
		ptr->regs[0] = 0;
		ptr->sp            = p->stack + THREAD_SIZE;
	}
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)ptr;
	p->pid            = nr_tasks;
	task[nr_tasks++]  = p;
	task[nr_tasks-1]->next = task[0];
	if(nr_tasks >=2)
		task[nr_tasks-2]->next = task[nr_tasks-1];
	preempt_enable();
	printf("%s leave\n",__func__);
	return 0;
}

int move_to_user_mode(unsigned long pc)
{
	printf("move_to_user_mode enter\n");
	struct pt_regs *regs = get_task_pt_regs(current);
	memzero((unsigned long)regs, sizeof(*regs));
	regs->pc = pc;
	regs->pstate = PSR_MODE_EL0t;
	unsigned long stack = get_free_page(); //allocate new user stack
	if (!stack) {
		return -1;
	}
	regs->sp = stack + PAGE_SIZE;
	current->stack = stack;
	printf("%s 1\n",__func__);
	return 0;
}
