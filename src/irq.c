#include "peripherals/irq.h"
#include "timer.h"
#include "io.h"
#include "printf.h"
#include "sysregs.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32",
	"SYNC_ERROR",
	"SYSCALL_ERROR",
	"DATA_ABORT_ERROR"
};

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	printf("%s, ESR: 0x%x, EC:0x%x,ISS:0x%x ELR: 0x%x\r\n", entry_error_messages[type], esr, esr >> 26, esr & ((1 << 25) -1), address);
}

void handle_irq(void)
{
	int currentEL = read_sysreg(CurrentEL) >> 2;
	printf("%s enter currentEL:%d\n",__func__, currentEL);
    unsigned int irq = readl(CORE0_IRQ_SOURCE);
    switch(irq)
    {
        case 2:
             handle_timer_irq();
             break;
    }
	printf("%s leave\n",__func__);
}
