#include "manager.h"
#include "ao.h"
#include "message.h"
#include "in_use.h"
#include "activeobject.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
/*********************************/
/* message queue access */
AO_MUTEX_T queue_access;
/*
#if defined (PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
	Mutex_Init(&queue_access);
#else
	Mutex_Init(&queue_access);
#endif
*/
/* conditonal to wake thread */
SIGNAL_T wake_thread;

int active_count = 0;

int empty_list = 0;

list active_tasks;
mlist message_queue;
/********************************/
#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)

	#include <windows.h>
#elif defined(PLATFORM_LINUX)
	#include <cpufreq.h>
#else
	#include <unistd.h>
#endif

int get_num_cores(){

#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;

#else


	return sysconf(_SC_NPROCESSORS_ONLN);	
#endif

}
#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKERPC)
	static THREAD_RETURN_T __stdcall thread_loop(void *args)
#else
	 static THREAD_RETURN_T thread_loop(void *args)
#endif
{
	message m;
	object o;
	int status;
	int kill_worker;
	/* worker thread loop */
	while(1){
	/*get access to message queue */
	Lock_Mutex(&queue_access);
	
	}
	
	return 0;

} /* end of thread_loop */


int init_manager(int initial_thread_count){
	int startup;
	int thread_count = 0;
	
	AO_THREAD_T worker;
	
	/* intialise list and message queue */
	active_tasks = init_in_list();
	message_queue = init_list();
	
	/*create initial threads*/
	initial_thread_count = get_num_cores();
	for(startup = 0; startup < initial_thread_count; startup++){
		Create_Thread(&worker,NULL,thread_loop,2);
		thread_count++;
	}
	
	if (thread_count != initial_thread_count){
		/* ERROR: they must be equal to continue */
		return AO_MANAGET_INIT_FAIL;
	}
	return AO_MANAGET_INIT;


}



int create_worker(void){

	AO_THREAD_T worker;
	Create_Thread(&worker,NULL,thread_loop,2);
	
	return AO_MANAGET_INIT;

}
