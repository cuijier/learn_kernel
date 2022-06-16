#include "vector_entry.h"
#include "asm-offsets.h"
#include "sysregs.h"
#include "sys.h"

.macro handle_invalid_entry el, type
kernel_entry \el
mov	x0, #\type
mrs	x1, esr_el1
mrs	x2, elr_el1
bl	show_invalid_entry_message
b	err_hang
.endm


/*
   保存异常发生时候的上下文
   保存x0~x29，x30（lr），sp, elr, spsr保存到 栈中
 */
	.macro kernel_entry, el
	/*
	   SP指向了栈底, S_FRAME_SIZE表示一个栈框的大小.
	   定义一个struct pt_regs来描述一个栈框,
	   用在异常发生时保存上下文.
	 */
	sub sp, sp, #S_FRAME_SIZE
	/*
	   保存通用寄存器x0~x29到栈框里pt_regs->x0~x29
	 */
	stp x0, x1, [sp, #16 *0]
	stp x2, x3, [sp, #16 *1]
	stp x4, x5, [sp, #16 *2]
	stp x6, x7, [sp, #16 *3]
	stp x8, x9, [sp, #16 *4]
	stp x10, x11, [sp, #16 *5]
	stp x12, x13, [sp, #16 *6]
	stp x14, x15, [sp, #16 *7]
	stp x16, x17, [sp, #16 *8]
	stp x18, x19, [sp, #16 *9]
	stp x20, x21, [sp, #16 *10]
	stp x22, x23, [sp, #16 *11]
	stp x24, x25, [sp, #16 *12]
	stp x26, x27, [sp, #16 *13]
	stp x28, x29, [sp, #16 *14]
	str x30, [sp, #S_X30]
	mrs     x22, elr_el1
	mrs     x23, spsr_el1
	.if	\el == 0
	mrs	x24, sp_el0
	.else
	add	x24, sp, #S_FRAME_SIZE
	.endif /* \el == 0 */
	str	x22, [sp, #S_PC]				// returned x0
	str	x23, [sp, #S_PSTATE]
	str	x24, [sp, #S_SP]
	.endm

/*
   恢复异常发生时保存下来的上下文
 */
	.macro kernel_exit, el
	/* 从pt_regs->pc中恢复elr_el1,
	   从pt_regs->pstate中恢复spsr_el1
	 */
	ldr     x22, [sp, #S_PSTATE]           // load SPSR
	msr     spsr_el1, x22
	ldr     x21, [sp, #S_PC]           // load SPSR
	msr     elr_el1, x21                    // set up the return data

	ldr     x23, [sp, #S_SP]           // load SPSR
	.if	\el == 0
	msr	sp_el0, x23
	.endif /* \el == 0 */
	ldp     x0, x1, [sp, #16 * 0]
	ldp     x2, x3, [sp, #16 * 1]
	ldp     x4, x5, [sp, #16 * 2]
	ldp     x6, x7, [sp, #16 * 3]
	ldp     x8, x9, [sp, #16 * 4]
	ldp     x10, x11, [sp, #16 * 5]
	ldp     x12, x13, [sp, #16 * 6]
	ldp     x14, x15, [sp, #16 * 7]
	ldp     x16, x17, [sp, #16 * 8]
	ldp     x18, x19, [sp, #16 * 9]
	ldp     x20, x21, [sp, #16 * 10]
	ldp     x22, x23, [sp, #16 * 11]
	ldp     x24, x25, [sp, #16 * 12]
	ldp     x26, x27, [sp, #16 * 13]
	ldp     x28, x29, [sp, #16 * 14]
	ldr     x30, [sp, #S_X30]           // load SPSR
	add     sp, sp, #S_FRAME_SIZE           // restore sp
	eret                                    // return to kernel
	.endm


	.macro	ventry	label
	.align	7
	b	\label
	.endm

/********************************************/
/***************** vectors ******************/
.align 11
.globl vectors
vectors:
		ventry	sync_invalid_el1t			// Synchronous EL1t
        ventry	irq_invalid_el1t			// IRQ EL1t
        ventry	fiq_invalid_el1t			// FIQ EL1t
        ventry	error_invalid_el1t			// Error EL1t

        ventry	sync_invalid_el1h			// Synchronous EL1h
        ventry	el1_irq						// IRQ EL1h
        ventry	fiq_invalid_el1h			// FIQ EL1h
        ventry	error_invalid_el1h			// Error EL1h

        ventry	el0_sync			// Synchronous 64-bit EL0
        ventry	el0_irq						// IRQ 64-bit EL0
        ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
        ventry	error_invalid_el0_64			// Error 64-bit EL0

        ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
        ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
        ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
        ventry	error_invalid_el0_32			// Error 32-bit EL0

tsk     .req    x28             // current thread_info

.macro get_thread_info, rd
mov     \rd, sp
//add     \rd, \rd, #S_FRAME_SIZE
and     \rd, \rd, #~(1<<12 - 1)   // top of stack
.endm

sync_invalid_el1t:
	handle_invalid_entry  1, SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  1, IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  1, FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  1, ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  1, SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  1, FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  1, ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  0, SYNC_INVALID_EL0_64

el0_irq:
	kernel_entry 0 
	bl	handle_irq
	b	ret_to_user

fiq_invalid_el0_64:
	handle_invalid_entry  0, FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  0, ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  0, SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  0, IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  0, FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  0, ERROR_INVALID_EL0_32


el1_irq:
	kernel_entry 1
	bl	handle_irq
	get_thread_info tsk
	ldr  w24, [tsk, #TIF_PREEMPT_COUNT]
	cbnz w24, 1f
	ldr  w0, [tsk, #THREAD_FLAG]
	//and  w0, w0, #_TIF_NEED_RESCHED
	cbz  w0, 1f
	bl el1_preempt
1:
	kernel_exit 1

el1_preempt:
	mov     x24, lr
	bl preempt_schedule_irq
	ret     x24
	//kernel_exit 

.globl err_hang
err_hang: b err_hang



el0_sync:
	kernel_entry 0
	mrs	x25, esr_el1				// read the syndrome register
	lsr	x24, x25, #ESR_ELx_EC_SHIFT		// exception class
	cmp	x24, #ESR_ELx_EC_SVC64			// SVC in 64-bit state
	b.eq	el0_svc
	cmp	x24, #ESR_ELx_EC_DABT_LOW		// data abort in EL0
	b.eq	el0_da
	handle_invalid_entry 0, SYNC_ERROR

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

el0_da:
	bl	enable_irq
	mrs	x0, far_el1
	mrs	x1, esr_el1
	bl	do_mem_abort
	cmp x0, 0
	b.eq 1f
	handle_invalid_entry 0, DATA_ABORT_ERROR
1:
	bl disable_irq				
	kernel_exit 0

work_pending:
	mov	x0, sp				// 'regs'
	bl	do_notify_resume
	b	finish_ret_to_user

.global ret_to_user
/*
 进程fork之后第一次进程切换
 对于内核线程：
    x19保存了进程回调函数的入口
    x20保存进程的回调函数的参数
 */
.align 2
.global ret_from_fork
ret_from_fork:
	bl schedule_tail
	cbz	x19, ret_to_user			// not a kernel thread
	mov x0, x20
	blr x19
ret_to_user:
	bl disable_irq
	get_thread_info tsk
	ldr	x1, [tsk, #THREAD_FLAG]
	//and	x2, x1, #_TIF_WORK_MASK
	mov x2, x1
	cbnz	x2, work_pending
finish_ret_to_user:
	ldr x1, [sp, #S_X8]
	str x1, [sp, #S_X0]
	kernel_exit 0

/*
进程切换： 保存prev进程的上下文，并且恢复next进程
的上下文
  cpu_switch_to(struct task_struct *prev,
	   struct task_struct *next);

需要保存的上下文： x19 ~ x29， sp， lr
保存到进程的task_struct->cpu_context
 */
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

