#include "ao.h"
#include "messagequeue.h"
#include "init.h"
#include <stdlib.h>
#include <stdio.h>

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

#elif defined (PLATFORM_LINUX)
/*	int i = o;
	while(!cpufreq_cpu_exists(++i))
	return i;
		
*/
	return sysconf(_SC_NPROCESSORS_ONLN);	
#endif

}



/* number of threads to initially create */
#define thread_count 5
/*int thread_count = get_num_cores();*/

AO_THREAD_T init_thrds[thread_count];
