#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "defs.h"

/* Check the return value of a syscall, on fail value print a message to stderr and exit, on success value return the value
	@param call_return: value return by the syscall
	@param msg_on_fail: message to print before errno if syscall failed
	@return: call_return
	@throws: EXIT_FAILURE
*/
int try_sys_call(int call_return, char msg_on_fail[]) {
	if (call_return >= 0) return call_return;
	perror(msg_on_fail);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
   int cars[22] = {44, 6, 5, 7, 3, 33, 19, 77, 11, 27, 26, 55, 14, 22, 9, 12, 20, 30, 8, 21, 31, 94}; // thx nico :D
}
