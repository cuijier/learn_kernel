#include "peripherals/pl_uart.h"
#include "sysregs.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("current EL is ");
	u64 currentEL = read_sysreg(CurrentEL) >> 2;
        uart_send(currentEL + '0');
        uart_send('\n');
	timer_init();
	enable_interrupt_controller();
	enable_irq();
	while (1) {
		uart_send(uart_recv());
	}
}
