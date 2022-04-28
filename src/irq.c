#include "peripherals/irq.h"
#include "timer.h"
#include "io.h"
void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	//printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}


void enable_interrupt_controller()
{
	writel(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
}


void handle_irq(void)
{
    unsigned int irq = readl(IRQ_PENDING_1);
    switch(irq)
    {
        case SYSTEM_TIMER_IRQ_1:
             handle_timer_irq();
             break;
    }
}
