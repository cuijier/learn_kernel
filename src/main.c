#include "peripherals/pl_uart.h"
#include "sysregs.h"
#include "uart.h"
#include "printf.h"
#include "current.h"
#include "sched.h"

extern struct task_struct* task[64];
static void delay(int n)
{
	while (n--)
		;
}

void kernel_thread(char *array)
{
	while (1) {
		delay(200000);
		printf("%s: %s [counter:%d, preempt:%d, need_resched:%d]\n", __func__, array, current->counter,  current->preempt_count, current->flag);
	}
}

void kernel_main(void)
{
	uart_init();
	u64 currentEL = read_sysreg(CurrentEL) >> 2;
	printf("current Exception level:%d\n", (int)currentEL);
	timer_init();
	enable_irq();
	int nRet = do_fork(0, (unsigned long)&kernel_thread, (unsigned long)"12345");
	    nRet = do_fork(0, (unsigned long)&kernel_thread, (unsigned long)"abcde");
	switch_to(task[0]);
	schedule();
	while(1);
}
