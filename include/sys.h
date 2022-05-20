#ifndef	_SYS_H
#define	_SYS_H

#define __NR_syscalls	    1

#define SYS_WRITE_NUMBER    0 		// syscal numbers 

#ifndef __ASSEMBLER__

void sys_write(char * buf);
void call_sys_write(char * buf);

#endif
#endif  /*_SYS_H */
