#ifndef	_USER_H
#define	_USER_H

void user_process1(char *array);
void user_process();
extern unsigned long user_begin;
extern unsigned long user_end;
void call_sys_write(char * buf);
int call_sys_clone(unsigned long fn, unsigned long arg, unsigned long stack);
unsigned long call_sys_malloc();
void call_sys_exit();
#endif  /*_USER_H */
