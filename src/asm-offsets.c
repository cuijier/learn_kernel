#include "sched.h"
#include <stddef.h>

#define DEFINE(sym, val) \
    asm volatile("\n->" #sym " %0 " #val : : "i" (val))

int main(void)
{
	DEFINE(S_TASK_SIZE,         sizeof(struct task_struct));
	DEFINE(S_FRAME_SIZE,        sizeof(struct pt_regs));
	DEFINE(THREAD_CPU_CONTEXT,	offsetof(struct task_struct, cpu_context));
	DEFINE(TIF_PREEMPT_COUNT,	offsetof(struct task_struct, preempt_count));
	DEFINE(THREAD_FLAG,			offsetof(struct task_struct, flag));
	DEFINE(S_X0,                offsetof(struct pt_regs, regs));
	DEFINE(S_X8,                offsetof(struct pt_regs, regs[8]));
	DEFINE(S_X30,                offsetof(struct pt_regs, regs[30]));
	DEFINE(S_SP,                offsetof(struct pt_regs, sp));
	DEFINE(S_PC,                offsetof(struct pt_regs, pc));
	DEFINE(S_PSTATE,            offsetof(struct pt_regs, pstate));
	DEFINE(S_PGD,               offsetof(struct task_struct, mm));
}