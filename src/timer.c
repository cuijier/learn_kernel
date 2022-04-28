#include "io.h"
//#include "printf.h"
#include "peripherals/timer.h"

const unsigned int interval = 20000;
unsigned int curVal = 0;

void timer_init ( void )
{
	curVal = readl(TIMER_CLO);
	curVal += interval;
	writel(TIMER_C1, curVal);
}

void handle_timer_irq( void ) 
{
	curVal += interval;
	writel(TIMER_C1, curVal);
	writel(TIMER_CS, TIMER_CS_M1);
	//printf("Timer interrupt received\n\r");
	uart_send_string("irq handle --\n");
}

