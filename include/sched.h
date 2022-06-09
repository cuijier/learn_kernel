#ifndef _SCHED_H
#define _SCHED_H
#include "mm.h"
#include "list.h"

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

enum task_state {
    TASK_RUNNING = 0,
    TASK_INTERRUPTIBLE = 1,
    TASK_UNINTERRUPTIBLE = 2,
    TASK_ZOMBIE = 3,
    TASK_STOPPED = 4,
};

#define TIF_SIGPENDING		0
#define TIF_NEED_RESCHED	1

#define _TIF_SIGPENDING		(1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1 << TIF_NEED_RESCHED)
#define _TIF_WORK_MASK       (_TIF_NEED_RESCHED)

#define PREEMPT_BITS	8
#define HARDIRQ_BITS	4

#define PREEMPT_SHIFT	0
#define HARDIRQ_SHIFT	(PREEMPT_SHIFT + PREEMPT_BITS)

#define PREEMPT_OFFSET	(1UL << PREEMPT_SHIFT)
#define HARDIRQ_OFFSET	(1UL << HARDIRQ_SHIFT)

#define TASK_SLICE         20
#define PF_KTHREAD                 0x00000002

#define MAX_PROCESS_PAGES			16	

struct user_page {
	unsigned long phys_addr;
	unsigned long virt_addr;
};

struct mm_struct {
	unsigned long pgd;
	int user_pages_count;
	struct user_page user_pages[MAX_PROCESS_PAGES];
	int kernel_pages_count;
	unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct task_struct {
	struct cpu_context   cpu_context;
	enum   task_state    state;	
	long   counter;
	long   preempt_count;
	int    flag;
    int    pid;
	void*  stack;
	struct mm_struct mm;
	struct task_struct * next;
};

struct pt_regs{
	unsigned long regs[31];
	unsigned long sp;
	unsigned long pc;
	unsigned long pstate;
};


union task_union {
	struct task_struct task;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};
#ifndef __ASSEMBLER__
void preempt_disable(void);
void preempt_enable(void);
void exit_process();
struct pt_regs * get_task_pt_regs(struct task_struct * task);
void schedule(void);
#endif
#endif
