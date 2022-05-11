#include "peripherals/pl_uart.h"
#include "sysregs.h"
#include "uart.h"
#include "printf.h"


static void delay(int n)
{
	while (n--)
		;
}

void kernel_thread(char *array)
{
	while (1) {
		delay(10000000);
		printf("%s: %s\n", __func__, array);
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
	schedule();
	while(1);
}
