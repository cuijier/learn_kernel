#ifndef	_TIMER_H
#define	_TIMER_H

void timer_init ( void );
void handle_timer_irq ( void );

/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define CONFIG_HZ       1
#define HZ CONFIG_HZ	/* Internal kernel timer frequency */

#endif  /*_TIMER_H */

