#include "sched.h"
#include "current.h"
#include "printf.h"

extern int nr_tasks;
extern struct task_struct* task[64];

int task_tick()
{
    //printf("%s enter\n",__func__);

    current->counter++;
    if(current->counter < TASK_SLICE)
    {
        //printf("%s leave1\n",__func__);
        return 0;
    }
    current->flag |= _TIF_NEED_RESCHED;
    //printf("%s leave2\n",__func__);
    return 0;
}

static inline void preempt_disable(void)
{
	current->preempt_count++;
}

static inline void preempt_enable(void)
{
	current->preempt_count--;
}

static inline void preempt_add(int val)
{
	current->preempt_count += val;
}

static inline void preempt_sub(int val)
{
	current->preempt_count -= val;
}

/*
 * 处理调度完成后的一些收尾工作，由next进程来收拾
 * prev进程的烂摊子
 *
 * Note: 新创建进程第一次运行也会调用该函数来处理
 * prev进程的烂摊子
 * ret_from_fork->schedule_tail
 */
void schedule_tail(struct task_struct *prev)
{
    //printf("%s enter\n",__func__);
    prev->counter = 0;
    prev->flag &= (~_TIF_NEED_RESCHED);
	/* 打开中断 */
	enable_irq();
    //printf("%s leave\n",__func__);
}

// preempt_count add 1<<8
#define __irq_enter() preempt_add(HARDIRQ_OFFSET)
// preempt_count sub 1<<8
#define __irq_exit() preempt_sub(HARDIRQ_OFFSET)


struct task_struct *pick_next_task(struct task_struct* rq[], int nr_tasks)
{
    return current->next;
}

void __schedule()
{
    //printf("%s enter\n",__func__);
    //disable_irq();
    struct task_struct *prev, *next, *last;
    prev = current;
    next = pick_next_task(task, nr_tasks);
    if(prev != next)
    {
        //printf("%s next:0x%x\n",__func__, next);
        //prev->counter = TASK_SLICE;
        last = cpu_switch_to(prev, next);
        //printf("%s last:0x%x\n",__func__, last);
    }
    schedule_tail(last);
}

void switch_to(struct task_struct *next)
{
	struct task_struct *prev = current;

	if (current == next)
		return;
	cpu_switch_to(prev, next);
}


/* 普通调度 */
void schedule(void)
{
	/* preempt_count >0, disable preempt */
	preempt_disable();
	__schedule();
    /* preempt_count =0, enable preempt */
	preempt_enable();
}


/* 抢占调度
 *
 * 中断返回前会检查是否需要抢占调度
 */
void preempt_schedule_irq(void)
{
    //printf("%s enter\n",__func__);
	/* 关闭抢占，以免嵌套发生调度抢占*/
	preempt_disable();
    enable_irq();
	__schedule();
	disable_irq();
	preempt_enable();
    //printf("%s leave\n",__func__);
}



