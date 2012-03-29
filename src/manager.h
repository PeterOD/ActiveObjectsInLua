#ifndef MANAGER_H
#define MANAGER_H
#include "ao.h"

#define AO_MANAGET_INIT 0
#define AO_MANAGET_INIT_FAIL -1
#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKERPC)
	static THREAD_RETURN_T __stdcall thread_loop(void *args);
#else
	 static THREAD_RETURN_T thread_loop(void *args);
#endif


int init_manager(int initial_thread_count);

int create_worker(void);


#endif