#include "user.h"
#include "printf.h"

extern void delay(int n);
void user_process1(char *array)
{
	while (1){
			call_sys_write("user_process1 enter \n");
			delay(200000);
		}
}

void user_process()
{
	//char buf[30] = {0};
	//sprintf(buf, "User process started\n");
	call_sys_write("User process started\n");
	//void * pstack1 = (void *)call_sys_malloc();
	// int nRet = call_sys_clone(user_process1, "12345", pstack1);
	// if(nRet < 0)
	// {
	// 	sprintf(buf, "call_sys_clone1 fail\n");
	// 	call_sys_write(buf);
	// }

	// void * pstack2 = (void *)call_sys_malloc();
	// nRet = call_sys_clone(user_process1, "abcde", pstack2);
	// if(nRet < 0)
	// {
	// 	sprintf(buf, "call_sys_clone2 fail\n");
	// 	call_sys_write(buf);
	// }
	call_sys_exit();
}