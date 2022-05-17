	.file	"asm-offsets.c"
	.text
	.globl	main
	.type	main, @function
main:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 9 "src/asm-offsets.c" 1
	
->THREAD_CPU_CONTEXT $0 offsetof(struct task_struct, cpu_context)
# 0 "" 2
# 10 "src/asm-offsets.c" 1
	
->TIF_PREEMPT_COUNT $120 offsetof(struct task_struct, preempt_count)
# 0 "" 2
# 11 "src/asm-offsets.c" 1
	
->TIF_NEED_RESCHED $128 offsetof(struct task_struct, flag)
# 0 "" 2
#NO_APP
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 8.4.0-3ubuntu2) 8.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
