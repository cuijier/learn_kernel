#include "sched.h"

void do_notify_resume(struct pt_regs *regs, unsigned long thread_flags)
{
        if (thread_flags & _TIF_NEED_RESCHED) {
        schedule();
    }
}