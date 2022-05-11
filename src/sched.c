#include "sched.h"
#include "current.h"

extern int nr_tasks;
extern struct task_struct* task[64];

int task_tick()
{
    current->counter--;
    if(current->counter > 0)
        return 0;
    current->counter = 0;
    current->flag |= _TIF_NEED_RESCHED;
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
	/* 打开中断 */
	enable_irq();
}

// preempt_count add 1<<8
#define __irq_enter() preempt_add(HARDIRQ_OFFSET)
// preempt_count sub 1<<8
#define __irq_exit() preempt_sub(HARDIRQ_OFFSET)


struct task_struct *pick_next_task(struct task_struct* rq[], int nr_tasks)
{
    int index = 0, max_counter = 0;
    for(int i = 0; i < nr_tasks; i++)
    {
        if(rq[i]->counter > max_counter)
        {
            max_counter = rq[i]->counter;
            index = i;
        }
    }
    return rq[index];
}

void __schedule()
{
    disable_irq();
    struct task_struct *prev, *next, *last;
    prev = current;
    next = pick_next_task(task, nr_tasks);
    if(prev != next)
    {
        last = cpu_switch_to(prev, next);
    }
    schedule_tail(last);
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
	/* 关闭抢占，以免嵌套发生调度抢占*/
	preempt_disable();
    enable_irq();
	__schedule();
	disable_irq();
	preempt_enable();
}



