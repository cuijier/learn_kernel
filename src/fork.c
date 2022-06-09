#include "sched.h"
#include "fork.h"
#include "current.h"
#include "printf.h"
#include "utils.h"
#include "mm.h"

int nr_tasks = 0;
struct task_struct* task[32] = {0,};

int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack)
{
	preempt_disable();
	struct task_struct *p = (struct task_struct *)allocate_kernel_page();
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
		memcpy((unsigned long)ptr, cur_regs, sizeof(struct pt_regs));
		ptr->regs[0] = 0;
		ptr->regs[8] = arg;
		ptr->sp            = (unsigned long)p->stack + THREAD_SIZE;
		ptr->pc            = fn;
		copy_virt_memory(p);
	}
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)ptr;
	p->pid            = nr_tasks;
	task[nr_tasks++]  = p;
	task[nr_tasks-1]->next = task[0];
	if(nr_tasks >=2)
		task[nr_tasks-2]->next = task[nr_tasks-1];
	printf("%s, task:%p, fn:%p, arg:%s, pgd:0x%lx\n", __func__, p, fn, arg, p->mm.pgd);
	preempt_enable();
	return 0;
}

int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc)
{
	printf("%s enter start:%p, size:%d, pc:%p\n",__func__, start, size, pc);
	struct task_struct *pCurrent = current;
	struct pt_regs *regs = get_task_pt_regs(pCurrent);
	regs->pstate = PSR_MODE_EL0t;
	regs->pc = pc;
	regs->sp = 2 *  PAGE_SIZE;
	//pCurrent->mm.pgd = get_pgd();
	void* code_page = allocate_user_page(pCurrent, 0);
	allocate_user_page(pCurrent, PAGE_SIZE);
	if (code_page == 0)	{
		return -1;
	}
	memcpy((unsigned long)code_page, start, size);
	set_pgd(pCurrent->mm.pgd);
	printf("%s leave\n",__func__);
	return 0;
}
