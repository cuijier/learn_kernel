#ifndef _FORK_H
#define _FORK_H

extern void ret_from_fork(void);
int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg);

#endif