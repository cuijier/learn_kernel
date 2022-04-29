#include "peripherals/irq.h"
#include "timer.h"
#include "io.h"
void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	//printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

void handle_irq(void)
{
    unsigned int irq = readl(0x40000060);
    switch(irq)
    {
        case 2:
             handle_timer_irq();
             break;
    }
}
