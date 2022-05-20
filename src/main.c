#include "peripherals/pl_uart.h"
#include "sysregs.h"
#include "uart.h"
#include "printf.h"
#include "current.h"
#include "sched.h"

extern struct task_struct* task[64];

void user_process()
{
	char buf[30] = {0};
	sprintf(buf, "User process started\n");
	call_sys_write(buf);
}

void kernel_thread(char *array)
{
	printf("kernel_thread enter\n");
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
