#include "io.h"
//#include "printf.h"
#include "timer.h"
#include "peripherals/timer.h"
#include "peripherals/irq.h"
#define  HZ                   250
static unsigned int val = NSEC_PER_SEC / HZ;

/*
*cntp_ctl_el0 : Control register for the EL1 physical timer
*        bit0 : 1 enable, 0 disable
*/
static int generic_timer_init(void)
{
	asm volatile(
		"mov x0, #1\n"
		"msr cntp_ctl_el0, x0"
		:
		:
		: "memory");
	return 0;
}


/*
*cntp_tval_el0 : Holds the timer value for the EL1 physical timer.
*/
static int generic_timer_reset(unsigned int val)
{
	asm volatile(
		"msr cntp_tval_el0, %x[timer_val]"
		:
		: [timer_val] "r" (val)
		: "memory");

	return 0;
}

static void enable_timer_interrupt(void)
{
	writel(SYSTEM_TIMER_IRQ_1, CORE0_TIMER_CTL);
}

void timer_init(void)
{
	generic_timer_init();
	generic_timer_reset(val);
	enable_timer_interrupt();
}

void handle_timer_irq(void)
{
	generic_timer_reset(val);
	uart_send_string("Core0 Timer interrupt received\r\n");
}

