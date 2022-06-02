### ARMv8 知识点

- **`ESR_ELn`**: 表示 `exception` 的触发条件.

![image](https://documentation-service.arm.com/static/5f872814405d955c5176de24?token=)

```
Bits [31:26] : 表示exception 触发的具体原因, 下面列出了部分原因.

EC = 0x15 : AArch64 下 svc 指令触发的.
EC = 0x16 : AArch64 下 hvc 指令触发的.
EC = 0x17 : AArch64 下 smc 指令触发的.
EC = 0x01 : WFI or WFE 指令触发的.
```
- **`SPSR_ELn`**: 状态寄存器.

```
M[3:0], bits [3:0]: exception 等级和 SP 选择,可选的值如下表.
        bit  [0] 0:使用的 SP0.
                 1:使用SP_ELn.
```

| M[3:0] | Meaning |
| --- | --- |
| 0b0000 | EL0t. |
| 0b0100 | EL1t. |
| 0b0101 | EL1h. |


---


### 系统调用的概念

操作系统的职责之一就是隔离各个用户空间的进程，使得各个用户空间进程相互独立且互不影响. 实现该目的的第一步就是权利分级，将用户空间进程全部运行在`EL0` 等级，这样用户进程就无法访问特权处理器. 如果不分级的话任何隔离技术都是无用的，因为任何一个用户进程都可以改写安全设置.

分级之后虽然从权限上分离了各个进程，但是也引入了不方便，比如用户进程想读写一个文件，或者想打印字符，就没有权限来做这些事情了.

对于运行在`EL0`等级的应用程序，可以调用`svc`指令来调用到`EL1`等级的内核API，这个调用动作通常都是封装在`glibc` 库中，应用程序基本上感知不到. 内核提供的能通过`svc`指令触发`exception` 来执行的`API` 并且执行完成之后自动返回到用户空间的称之为`系统调用`. 

![image](https://documentation-service.arm.com/static/5f872814405d955c5176de27?token=)

### 系统调用的实现
内核将提供给用户空间使用的系统调用API 组成一个数组, 数组大小就是提供的总系统调用个数，数组下标就是系统调用号. 这样用户空间只需要传递系统调用号就能完成系统调用.

```
#define __NR_syscalls	    1
#define SYS_WRITE_NUMBER    0 		// syscal numbers 
void * const sys_call_table[] = {sys_write};
```


将系统调用号通过`w8`寄存器传递，之所以从`w8` 开始，是因为 `x0`~`x7`预留给系统调用传参用，如果有多个参数，将参数依次加载到`x0` 到`x7`寄存器，然后调用`svc` 指令. 这步操作都是封装在`glibc` 中的，所以`Linux`上用户感知不到`svc`指令的调用.

```
.globl call_sys_write
call_sys_write:
	mov w8, #SYS_WRITE_NUMBER	
	svc #0
	ret
```

`svc` 调用会触发一个`同步异常`,异常来源是`EL0`,所以处理入口是 `el0_sync (Synchronous 64-bit EL0)`.

```
el0_sync:
	kernel_entry 0
	mrs	x25, esr_el1				// read the syndrome register
	lsr	x24, x25, #ESR_ELx_EC_SHIFT		// exception class
	cmp	x24, #ESR_ELx_EC_SVC64			// SVC in 64-bit state
	b.eq	el0_svc
	handle_invalid_entry 0, SYNC_ERROR
```
该函数中首先通过`kernel_entry` 宏在栈上保存现场, 然后读取`esr_el1` 寄存器的值到`x25`后右移 26bit 赋值给`x24`, `x24` 里面的值就是上文介绍的`EC`,判断 `EC` 值是否等于`0x15` 如果等于则说明该同步异常是由`svc` 指令触发的，则跳转到`el0_svc` 来响应`svc`.

`el0_svc`主要完成如下事情
- 将系统调用数组首地址加载到`x27`, 用户空间传递下来的系统调用号加载到`x26`,总系统调用数放到`x25`.
- 打开中断,检查系统调用号是否小于总系统调用个数，如果小于，则根据系统调用号得到对应的API 指针.
- 调用系统调用API.
- 将系统调用返回值从`cpu_content x0`加载到 `x0`寄存器，然后恢复到同步异常发生前的现场, 至此整个系统调用流程结束. 

```
sc_nr	.req	x25					// number of system calls
scno	.req	x26					// syscall number
stbl	.req	x27					// syscall table pointer

el0_svc:
	adr	stbl, sys_call_table			// load syscall table pointer
	uxtw	scno, w8				// syscall number in w8
	mov	sc_nr, #__NR_syscalls
	bl	enable_irq
	cmp     scno, sc_nr                     	// check upper syscall limit
	b.hs	ni_sys

	ldr	x16, [stbl, scno, lsl #3]		// address in the syscall table
	blr	x16					// call sys_* routine
	b	ret_from_syscall
ni_sys:
	handle_invalid_entry 0, SYSCALL_ERROR
ret_from_syscall:
	bl	disable_irq
	str	x0, [sp, #S_X0]				// returned x0
	kernel_exit 0
```

![4.3_sys_call.png](https://s2.loli.net/2022/05/26/RzT1wPDKdC3OotE.png)


### 用户进程
用户进程除了权限没有内核进程高，无法访问系统资源外，使用的栈空间也和内核进程相互独立，因为用户进程在执行系统调用时会陷入内核态，即存在内核态和用户态相互切换的情况.

到目前为止所有的进程都运行在内核态，那么用户进程怎么创建和运行呢 ? 有两种方式：将一个内核进程移到用户空间执行；用户进程fork 新进程，新创建的进程也是用户进程.

#### 将任务移至用户模式
内核进程移到用户空间的实现如下

```
int move_to_user_mode(unsigned long pc)
{
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
	return 0;
}
```
用到了`pt_regs`,主要将`pc`,`pstate`,`sp` 三个字段缓存在该结构体中.
- **`pc`**: 缓存的是用户态函数指针，在`kernel_exit` 中会将该值加载到`elr_el1`.
- **`pstate`**: 赋值为 0 ,`kernel_exit` 会将该值加载到`spsr_el1`,上文介绍可知赋值为0 后模式为`EL0t`.
- **`sp`**: 用户态栈顶的地址，在`kernel_exit` 中会加载到`sp_el0`，用户态使用的是`sp_el0` 栈指针.


![用户态进程.png](https://s2.loli.net/2022/05/24/lrNHXn35cIP1ba6.png)


整个调用流程如下图

![4_move_to_user.png](https://s2.loli.net/2022/05/25/RqC7dgfBcN5r8ph.png)


#### 用户进程clone调用
这里实现的`clone` 功能和 `thread_create`接口差不多，和`Linux` 的`sys_clone`有区别.  和其他系统调用一样，还是通过`w8` 传递系统调用号，同时传递了三个参数


```
int sys_clone(void *fn, void * arg, void * stack)
{
	return do_fork(0, fn ,arg, stack);
}
```

- 用户态函数指针作为第一个参数传递到`x0`.
- 函数入参作为第二个参数传递d到`x1`.
- 用户态栈指针作为第三个参数传递到`x2`.

在`do_fork` 中主要完成几件事情
1. 为子进程分配`task_struct` 结构体并初始化各成员变量的值.
2. 拷贝父进程的`pt_regs` ,将`pt_regs->pc`赋值为用户进程函数指针,`pt_regs->regs[8]`缓存参数指针.
3. 内核态`cpu_context.pc`赋值为`ret_from_fork`.
4. 将子进程添加到进程列表中.

```
int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack)
{
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
		memcpy(ptr, cur_regs, sizeof(struct pt_regs));
		ptr->regs[0] = 0;
		ptr->regs[8] = arg;
		ptr->sp            = p->stack + THREAD_SIZE;
		ptr->pc            = fn;
	}
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)ptr;
	p->pid            = nr_tasks;
	task[nr_tasks++]  = p;
	task[nr_tasks-1]->next = task[0];
	if(nr_tasks >=2)
		task[nr_tasks-2]->next = task[nr_tasks-1];
	preempt_enable();
	return 0;
}
```

当调度器第一次调度到子进程时会将子进程内核上下文`cpu_context` 加载到寄存器，`nret` 后就执行到了`ret_from_fork` 在该函数中通过`cpu_context.x19` 寄存器为空判断出当前子进程为用户进程，于是调用`ret_to_user` 函数返回用户空间, 在`kernel_exit` 函数中已经将`pt_regs` 加载到了系统寄存器，这样就实现了返回用户空间执行既定函数的目的.

![4.4_user_process_clone.png](https://s2.loli.net/2022/05/26/QXpuemhiIbFqDLP.png)








