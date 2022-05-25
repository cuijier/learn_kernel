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

void user_process1(char *array)
{
	char buf[30] = {0};
	while (1){
			sprintf(buf, "%s\n", array);
			call_sys_write(buf);
			delay(100000);
		}
}

void user_process()
{
	char buf[30] = {0};
	sprintf(buf, "User process started\n");
	call_sys_write(buf);
	void * pstack1 = (void *)call_sys_malloc();
	int nRet = call_sys_clone(user_process1, "12345", pstack1);
	if(nRet < 0)
	{
		sprintf(buf, "call_sys_clone1 fail\n");
		call_sys_write(buf);
	}

	void * pstack2 = (void *)call_sys_malloc();
	nRet = call_sys_clone(user_process1, "abcde", pstack2);
	if(nRet < 0)
	{
		sprintf(buf, "call_sys_clone2 fail\n");
		call_sys_write(buf);
	}
	call_sys_exit();
}

void kernel_thread(char *array)
{
	printf("kernel_thread enter\n");
	preempt_disable();
	int err = move_to_user_mode((unsigned long)&user_process);
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	}
}

void kernel_main(void)
{
	uart_init();
	u64 currentEL = read_sysreg(CurrentEL) >> 2;
	printf("current Exception level:%d\n", (int)currentEL);
	timer_init();
	enable_irq();
	int nRet = do_fork(PF_KTHREAD, (unsigned long)&kernel_thread, (unsigned long)"12345", 0);
	switch_to(task[0]);
	while(1);
}


void spprint(unsigned long sp, int flag)
{
	printf("sp:0x%p, flag:%d\n", sp,flag);
}
