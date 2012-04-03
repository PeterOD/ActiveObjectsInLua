#ifndef MANAGER_H
#define MANAGER_H
#include "ao.h"

#define AO_MANAGET_INIT 0
#define AO_MANAGET_INIT_FAIL -1



int init_manager(int initial_thread_count);

int create_worker(void);


#endif