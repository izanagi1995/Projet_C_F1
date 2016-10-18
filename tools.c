#include <stdio.h>
#include <stdlib.h>
#include "tools.h"

/* Check the return value of a syscall, on fail value print a message to stderr and exit, on success value return the value
    @param call_return: value return by the syscall
    @param msg_on_fail: message to print before errno if syscall failed
    @return: call_return
    @throws: EXIT_FAILURE
*/
int try_sys_call_int(int call_return, char msg_on_fail[]) {
    if (call_return != -1) return call_return;
    perror(msg_on_fail);
    exit(EXIT_FAILURE);
}

void* try_sys_call_pts(void* call_return, char msg_on_fail[]) {
    if (call_return != NULL) return call_return;
    perror(msg_on_fail);
    exit(EXIT_FAILURE);
}