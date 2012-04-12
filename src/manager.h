#ifndef MANAGER_H
#define MANAGER_H
#include "ao.h"
#include "in_use.h"




#define AO_MANAGET_INIT 0
#define AO_MANAGET_INIT_FAIL -1



int init_manager(int initial_thread_count);

int create_worker(void);

void object_count(void);

void object_count_incr(void);
void join_threads_to_exit(void);

#endif