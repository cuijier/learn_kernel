#include "peripherals/pl_uart.h"
#include "sysregs.h"
#include "uart.h"
#include "printf.h"
#include "current.h"
#include "sched.h"
#include "sys.h"
#include "user.h"

extern struct task_struct* task[64];

void delay(int n)
{
	while (n--)
		;
}

void kernel_thread(char *array)
{
	printf("%s enter\n",__func__);
	disable_irq();
	dump_backtrace(NULL, NULL);
	unsigned long begin = (unsigned long)&user_begin;
	unsigned long end = (unsigned long)&user_end;
	unsigned long process = (unsigned long)&user_process;
	int err = move_to_user_mode(begin, end - begin, process - begin);
	//enable_irq();
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	}
}

void kernel_process(char *array)
{
	while (1){
			printf("fn:%s, args:%s\n", __func__, array);
			delay(200000);
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
			   do_fork(PF_KTHREAD, (unsigned long)&kernel_process, (unsigned long)"kernel process", 0);
	switch_to(task[0]);
	while(1);
}


void spprint(unsigned long sp, int flag)
{
	printf("sp:0x%p, flag:%d\n", sp,flag);
}
