/*
	AO.H
*/

#ifndef AO_H
#define AO_H

/*
	Platform Detection
*/
#ifdef _WIN32_WCE
	#define PLATFORM_POCKETPC
#elif (defined _WIN32)
	#define PLATFORM_WIN32
#elif (defined __linux__)
	#define PLATFORM_LINUX
#elif (defined __CYGWIN__)
	#define PLATFORM_CYGWIN
#else
	#error "DOOMED!! UNKNOWN PLATFORM"
#endif

typedef int bool_t;
#ifndef FALSE
# define FALSE 0
# define TRUE 1
#endif

typedef unsigned int uint_t;

#if defined(PLATFORM_WIN32) && defined(__GNUC__)
	#define mktime _mktime32
#endif
#include <time.h>
/*#include<process.h>*/

/*
	Enumeration: status
	
	This is an enumeration of the status of threads
*/
	/* enum status {RUNNING,WAITING,DONE,ERROR_T}; */
	
/*
	==== LOCKS AND SIGNALS ===
*/

#if defined(PLATFORM_WIN32) || defined(PLATFORM_POCKETPC)
	#define WIN32_LEAN_AND_MEAN
	/*  SignalObjectAndWait needs the following for updating locks */
	#define _WIN32_WINNT 0x0400
	#include <windows.h>
	#include <process.h>
	
	/*
		Mutex locks are needed to use with SIGNALs on windows platforms
		
	*/
	#define AO_STATE lua_State
	#define AO_MUTEX_T HANDLE
	/*
		Function: Mutex_init
		
		Parameters: AO_MUTEX_T *ao
		
		This function will be used to initialise a mutex lock
		note: this is not used for setting a lock in lua code
		see <Lua_Set_lock(AO_State *s)>
	*/
	void Mutex_Init(AO_MUTEX_T *ao);
	#define MUTEX_RECURSIVE_INIT(ao) Mutex_Init(ao)
	
	/*
		Function: Free_Mutex(AO_MUTEX_T *ao)
		
		Parameters: AO_MUTEX_T *ao
		
		This function will release a mutex lock.
		Note: This is not used for releasing a lock in lua code
	*/
	void  Free_Mutex(AO_MUTEX_T *ao);
	
	/*
		Function: Lock_Mutex(AO_MUTEX_T *ao)
		
		Parameters: AO_MUTEX_T *ao
		
		This function will set a mutex lock.
		Note: This is not used for setting a lock in lua code
	*/
	void Lock_Mutex(AO_MUTEX_T *ao);
	
	/*
		Function: Unlock_Mutex(AO_MUTEX_T *ao)
		
		Parameters: AO_MUTEX_T *ao
		
		This function will unlock a mutex lock.
		Note: This is not used for unlocking a lock in lua code
	*/
	void Unlock_Mutex(AO_MUTEX_T *ao);
	
	typedef unsigned THREAD_RETURN_T;
	
	#define SIGNAL_T HANDLE
	#define YIELD() Sleep(0)
#else
	/*
		This is the POSIX specific section
	*/
	#include <pthread.h>
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
	
	#ifdef PLATFORM_LINUX
		#define _RECURSIVE_MUTEX PTHREAD_MUTEX_RECURSIVE_NP
	#endif
	
	#define AO_MUTEX_T pthread_mutex_t
	#define Mutex_Init(ref) pthread_mutex_init(ref,NULL)
	#define Mutex_Recursive_Init(ref) \
		{ pthread_mutexattr_t a; pthread_mutexattr_init(&a); \
		  pthread_mutexattr_settype(&a, _RECURSIVE_MUTEX); \
		  pthread_mutex_init(ref,&a); pthread_mutexattr_destroy( &a); \
		}
	#define Free_Mutex(ao)	pthread_mutex_destroy(ao)
	#define Lock_Mutex(ao)  pthread_mutex_lock(ao)
	#define Unlock_Mutex(ao) pthread_mutex_unlock(ao)
	
	/*
		Typedef: void* THREAD_RETURN_T
		
		Other: For pthreads this is a pointer to void
	*/
	typedef void* THREAD_RETURN_T;
	
	typedef pthread_cond_t SIGNAL_T;
	
	void Signal_One(SIGNAL_T *ao);
	
	/*yield is not portable*/
	
#endif
void Init_Signal( SIGNAL_T *ao );
void Free_Signal( SIGNAL_T *ao );
void All_Signal( SIGNAL_T *ao );

typedef double time_d;
time_d now_secs(void);

time_d SIGNAL_TIMEOUT_PREPARE( double rel_secs );

bool_t SIGNAL_WAIT( SIGNAL_T *ref, AO_MUTEX_T *mu, time_d timeout );



/*
	Section: Threading
	
	This section declares the functions for threading.
*/

#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)

	typedef HANDLE AO_THREAD_T;
	
	/*
		Function: Create_Thread
		
		Parameters: 
					AO_THREAD_T *ao
					THREAD_RETURN_T(*func)( void *) - pointer to function that contains 'task' to be executed
					void *data 
					int priority
					
		This function will create a thead.
		note - this prototype abstracts the win thread implementation
	*/
	
	void Create_Thread(AO_THREAD_T *ao, THREAD_RETURN_T(__stdcall *func)( void *),void *data, int priority);
	
	#define AO_MINIMUM_PRIORITY (-3)
	#define AO_MAXIMUN_PRIORITY (+3)
	
#else
	/*
		platforms that use pthread_join() otherwise use condition variable
	*/
	
#ifdef USE_PTHREAD_TIMEJOIN
	#define PTHREAD_TIMEJOIN pthread_timejoin_np
#endif
	
	typedef pthread_t AO_THREAD_T;
	
	/*
		Function: Create_Thread
		
		Parameters: 
					AO_THREAD_T *ao
					THREAD_RETURN_T(*func)( void *) - pointer to function that contains 'task' to be executed
					void *data 
					int priority
					
		This function will create a thead.
		note - this prototype abstracts the pthread implementation
	*/

	void Create_Thread(AO_THREAD_T *ao, THREAD_RETURN_T(*func)( void *),void *data, int priority);
	
#if defined(PLATFORM_LINUX)
	volatile bool_t s;
	#ifdef LINUX_SCHED_RR
		#define THREAD_PRIORITY_MINIMUM (s ? -2 :0)
	#else
		#define THREAD_PRIORITY_MINIMUM 0
	#endif
	#define THREAD_PRIORITY_MAX (s ? +2 :0)
#else
	#define THREAD_PRIORITY_MINIMUM (-2)
	#define THREAD_PRIORITY_MAX (+2)
#endif

/*	
	win32 and pthread_timedjoin allow waiting for a thread with a time out
	otherwise conditional variables must be used

*/

#if	(defined PLATFORM_WIN32) || (defined PLATFORM_POCKECTPC) || (defined PTHREAD_TIMEDJOIN)
	bool_t Wait_Thead(AO_THREAD_T *ao, double secs);
#else
	bool_t Wait_Thread(AO_THREAD_T *ao, SIGNAL_T *sig_ao, AO_MUTEX_T *mu, volatile enum status *status_ptr,double secs);
#endif
void Kill_Thread(AO_THREAD_T *ao);
#endif /* ==== END OF AO_H ===== */
#endif
