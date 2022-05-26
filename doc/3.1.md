### 前置知识点
##### 基本指令
- **`nret`**: 在当前运行等级下使用对应的`ELR_ELx`、`SPSR_ELx`寄存器返回, 调用后`PE`会自动将`SPSR_ELx`加载到`PSTATE`, 并跳转到`ELR` 执行.




#### 进程描述
如果我们想管理进程，必须将进程抽象成一个结构体来描述，目前只实现进程切换，没有涉及到进程的资源管理，所有进程结构体比较简单.定义 `struct task_struct` 来描述进程,如下.

```
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

struct task_struct {
	struct cpu_context   cpu_context;
	enum   task_state    state;	
	long   counter;
	long   preempt_count;
	int    flag;
    int    pid;
	struct task_struct * next;
};
```

各结构体解释如下
- `cpu_context`: 进程上下文, 不同的进程这些寄存器会不一样，所以需要放在进程结构体中，进程切换本质上就是该结构体值的切换.
- `state`: 表示进程的状态,后续调度器会用到.
- `counter`: 统计进程运行的时间，表示本进程运行了多少个 `tick` 周期.
- `preempt_count`: 进程抢占标志位，如果 >0，则禁止抢占当前进程.
- `flag`: 需要调度标志位，如果 >0 ，则表示本进程时间片用尽，可以被抢占.
- `pid`: 进程ID.
- `next`: 指向下一个进程.

除了多进程切换需要保存进程的上下文外，还有中断上下文需要缓存,当`exception` 发生时我们需要保存当前进程的状态，处理完中断后，恢复之前进程的状态后继续执行. 中断上下文结构体可以放到`task_struct`, 也可以单独，这里就不放进去，但是和`task_struct` 放在同一个`page`.

```
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
```

`task_struct` 和 `pt_regs` 位置如下图.

![3.1.png](https://s2.loli.net/2022/05/25/uhpBrdQZAcJ5V9X.png)


#### 进程创建
在启动主进程中, 调用`do_fork` 创建了两个进程. 然后使用了`switch_to` 接口切换到了第一个进程.

```
	int nRet = do_fork(0, (unsigned long)&kernel_thread, (unsigned long)"12345");
	    nRet = do_fork(0, (unsigned long)&kernel_thread, (unsigned long)"abcde");
	switch_to(task[0]);
```
`do_fork` 的具体实现如下.

```
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

.align 2
.global ret_from_fork
ret_from_fork:
	bl schedule_tail
	mov x0, x20
	blr x19
```
在`do_fork` 函数中，首先分配了一个`page` 的内存，将page 首地址转换为 `task_struct` 地址, 然后对`task_struct` 赋初值.
- `count` = 0 : 表示新创建的这个进程还没有开始执行，执行时间为0 个tick 周期.
- `preempt_count`=0 : 初始状态为允许抢占.
- `flag`=0: 新进程初始状态为不需要被调度出去.
- `cpu_context.x19`=fn : 新创建进程的函数指针保存在 x19.
- `cpu_context.x20`=arg: 新创建进程的入参指针.
- `cpu_context.pc`=`ret_from_fork`: 新创建进程第一次调度开始执行的函数, 该函数处理完老进程残留的事情后，通过`blr` 指令跳转执行新进程的函数.
- `cpu_context.sp`=`p + THREAD_SIZE`: 初始化栈指针，指向新进程`page` 末尾.

所有新建进程的`task_struct ` 指针放到 全局数组`task`中.



#### 当前进程
在进程管理中经常需要获取当前进程的`task_struct` ,所以定义如下宏函数用来获取当前进程结构体.

```
register unsigned long current_stack_pointer asm ("sp");

static inline struct task_struct *get_current(void)
{
	return (struct task_struct *)
		(current_stack_pointer & ~(THREAD_SIZE - 1));
}

#define current get_current()
```
就是使用`sp` 寄存器 - `THREAD_SIZE`得到当前进程`page` 的首地址，即`task_struct`的地址.


#### 进程调度
进程调度分为两种情况，主动调度和被动调度.

##### 1. 主动调度
由于当前进程不满足继续执行的条件，进程可以主动调用`schedule`函数来让出CPU 执行别的进程，`schedule`实现如下.


```
void schedule(void)
{
	/* preempt_count >0, disable preempt */
	preempt_disable();
	__schedule();
    /* preempt_count =0, enable preempt */
	preempt_enable();
}


void __schedule()
{
    printf("%s enter\n",__func__);
    struct task_struct *prev, *next, *last;
    prev = current;
    next = pick_next_task(task, nr_tasks);
    if(prev != next)
    {
        printf("%s next:0x%x\n",__func__, next);
        last = cpu_switch_to(prev, next);
        printf("%s last:0x%x\n",__func__, last);
    }
    schedule_tail(last);
}
```
在主动调度过程中全程都是禁止抢占的，不然可能在主动调度中发生中又嵌套被动调度.  禁止抢占是通过`preempt_count` 标志位来实现的,`preempt_disable` 只是将该标志位+1, 具体起作用是在`el1_irq` 中断，中断里会获取 `task_struct`中 ``preempt_count`` 的计数, 如果是不等于0, 则退出中断.

```
el1_irq:
	kernel_entry
	bl	handle_irq
	get_thread_info tsk
	ldr  w24, [tsk, #TIF_PREEMPT_COUNT]
	cbnz w24, 1f
	ldr  w0, [tsk, #TIF_NEED_RESCHED]
	//@ and  w0, w0, #_TIF_NEED_RESCHED
	cbz  w0, 1f
	bl el1_preempt
1:
	kernel_exit
```
主动调度禁止抢占，但是是允许中断的，此时中断还是`Enable` 状态.
在进程数组中选择下一个运行的进程，现在选择策略很简单，就是运行当前进程的下一个进程，运行到最后一个进程后又从最开始的进程开始执行. 

```
struct task_struct *pick_next_task(struct task_struct* rq[], int nr_tasks)
{
    return current->next;
}
```
不同进程上下文切换实现如下，有两个入参

```
.align
.global cpu_switch_to
cpu_switch_to:
	add     x8, x0, #THREAD_CPU_CONTEXT
	mov     x9, sp
	stp     x19, x20, [x8], #16
	stp     x21, x22, [x8], #16
	stp     x23, x24, [x8], #16
	stp     x25, x26, [x8], #16
	stp     x27, x28, [x8], #16
	stp     x29, x9, [x8], #16
	str     lr, [x8]

	add     x8, x1, #THREAD_CPU_CONTEXT
	ldp     x19, x20, [x8], #16
	ldp     x21, x22, [x8], #16
	ldp     x23, x24, [x8], #16
	ldp     x25, x26, [x8], #16
	ldp     x27, x28, [x8], #16
	ldp     x29, x9, [x8], #16
	ldr     lr, [x8]
	mov     sp, x9
	ret
```
`x0` 传入的是prev 进程指针, `x1` 传入的是`next` 进程指针, 这个函数就是将当前进程环境`x19` ~ `x29`,`sp`,`lr`寄存器值存入 `prev->cpu_context`，然后将 `next->cpu_context` 的值加载到`x19` ~ `x29`,`sp`,`lr`. 当`next` 第一次切换到时，此时 `lr=(unsigned long)ret_from_fork`, `ret` 指令执行后就执行到了`ret_from_fork`，在`ret_from_fork` 函数中会调用`schedule_tail` 函数来对 prev 进程收尾，然后`blr` 到`x19`, 这时`next` 进程就真的执行起来了.

```
void schedule_tail(struct task_struct *prev)
{
    prev->counter = 0;
    prev->flag &= (~_TIF_NEED_RESCHED);
	/* 打开中断 */
	enable_irq();
}
```
prev 进程收尾函数`schedule_tail` 这里主要是将`counter` 和调度标志位清0.


##### 2. 被动调度

被动调度不是进程主动让出CPU的，而是调度程序判断当前进程运行时间到达了一定的阈值后剥夺了当前进程继续执行的权利，切换到下一个更需要运行的进程来运行. 被动调度整个过程都是由`tick` 中断来驱动的, 当`tick` 中断触发后，会在中断函数中将当前进程运行计数`counter`+1, 如果运行时间达到了时间片阈值`TASK_SLICE`，则将`flag` 置位.


```
int task_tick()
{
    current->counter++;
    if(current->counter < TASK_SLICE)
    {
        return 0;
    }
    current->flag |= _TIF_NEED_RESCHED;
    return 0;
}
```

在中断函数中如果允许抢占，且`flag`置位，则调用`preempt_schedule_irq` 函数来调度进程.

```
void preempt_schedule_irq(void)
{
	preempt_disable();
    enable_irq();
	__schedule();
	disable_irq();
	preempt_enable();
}
```
 中断调度时同样需要禁止抢占以免发生抢占嵌套，同时允许中断嵌套.
 
![3.2.PNG](https://s2.loli.net/2022/05/25/nD8TCzljiM2rJW7.png)

