#include "vector_entry.h"
#include "asm-offsets.h"


	.macro handle_invalid_entry type
	kernel_entry
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
	.macro kernel_entry
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
	mrs     x22, elr_el1
	mrs     x23, spsr_el1
	/* 把lr保存到pt_regs->lr, 把sp保存到pt_regs->sp位置*/
	stp x30, x22, [sp, #16 *15]
	/* 把elr_el1保存到pt_regs->pc中
	   把spsr_elr保存到pt_regs->pstate中*/
	str x23, [sp, #16 *16]
	.endm

/*
   恢复异常发生时保存下来的上下文
 */
	.macro kernel_exit
	/* 从pt_regs->pc中恢复elr_el1,
	   从pt_regs->pstate中恢复spsr_el1
	 */
	ldr     x22, [sp, #16 *16]           // load ELR, SPSR
	ldp     x30, x21, [sp, #16 * 15]
	msr     elr_el1, x21                    // set up the return data
	msr     spsr_el1, x22
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
        ventry	el1_irq					// IRQ EL1h
        ventry	fiq_invalid_el1h			// FIQ EL1h
        ventry	error_invalid_el1h			// Error EL1h
        ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
        ventry	irq_invalid_el0_64			// IRQ 64-bit EL0
        ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
        ventry	error_invalid_el0_64			// Error 64-bit EL0
        ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
        ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
        ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
        ventry	error_invalid_el0_32			// Error 32-bit EL0


sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32


tsk     .req    x28             // current thread_info

	.macro get_thread_info, rd
	mov     \rd, sp
	and     \rd, \rd, #~(1<<12 - 1)   // top of stack
	.endm

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

el1_preempt:
	mov     x24, lr
	bl preempt_schedule_irq
	ret     x24
	//kernel_exit 

.globl err_hang
err_hang: b err_hang


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
	mov x0, x20
	blr x19


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

