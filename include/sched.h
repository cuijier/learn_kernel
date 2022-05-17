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

#define PREEMPT_BITS	8
#define HARDIRQ_BITS	4

#define PREEMPT_SHIFT	0
#define HARDIRQ_SHIFT	(PREEMPT_SHIFT + PREEMPT_BITS)

#define PREEMPT_OFFSET	(1UL << PREEMPT_SHIFT)
#define HARDIRQ_OFFSET	(1UL << HARDIRQ_SHIFT)

#define TASK_SLICE         20

struct task_struct {
	struct cpu_context   cpu_context;
	enum   task_state    state;	
	long   counter;
	long   preempt_count;
	int    flag;
    int    pid;
	struct task_struct * next;
};

struct pt_regs {
	unsigned long x0;
	unsigned long x1;
	unsigned long x2;
	unsigned long x3;
	unsigned long x4;
	unsigned long x5;
	unsigned long x6;
	unsigned long x7;
	unsigned long x8;
	unsigned long x9;
	unsigned long x10;
	unsigned long x11;
	unsigned long x12;
	unsigned long x13;
	unsigned long x14;
	unsigned long x15;
	unsigned long x16;
	unsigned long x17;
	unsigned long x18;
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
	unsigned long x29;
	unsigned long x30;
	unsigned long elr_el1;
	unsigned long spsr_el1;
};


union task_union {
	struct task_struct task;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};
#endif
