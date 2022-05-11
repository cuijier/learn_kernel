#include "sched.h"
#include <stddef.h>

#define DEFINE(sym, val) \
    asm volatile("\n->" #sym " %0 " #val : : "i" (val))

int main(void)
{
	DEFINE(THREAD_CPU_CONTEXT,	offsetof(struct task_struct, cpu_context));
	DEFINE(TIF_PREEMPT_COUNT,	offsetof(struct task_struct, preempt_count));
	DEFINE(TIF_NEED_RESCHED,	offsetof(struct task_struct, flag));
}