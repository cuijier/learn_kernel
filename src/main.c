#include "peripherals/pl_uart.h"
#include "sysregs.h"
#include "uart.h"
#include "printf.h"

void kernel_main(void)
{
	uart_init();
	u64 currentEL = read_sysreg(CurrentEL) >> 2;
	printf("current Exception level:%d\n", (int)currentEL);
	timer_init();
	enable_irq();
	while (1) ;
	//{
	//	uart_send(uart_recv());
	//}
}
