//
// Created by Nicolas Surleraux on 8/11/16.
//

#ifndef PROJET_C_F1_FUNC_H
#define PROJET_C_F1_FUNC_H

#include <semaphore.h>

int try_sys_call_int(int syscall_ret, char* msg_on_fail);
void* try_sys_call_ptr(void* syscall_ret, char* msg_on_fail);
void sighandler(int sig);
float doSector(pilote* p, float time, int pipe);
extern sem_t* sem;
void init_semaphore();
int flush_pipe(int pipe);
int compare_rank_item(const void* a, const void* b);
int compare_pilotes(const void* a, const void* b);
int compare_pilote_position(const void* a, const void* b);
extern volatile sig_atomic_t flag_alarm;
extern volatile sig_atomic_t flag_race_stop;

#endif //PROJET_C_F1_FUNC_H
