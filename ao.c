/*
Copyright (C) 2007-10 Asko Kauppi <akauppi@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "ao.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

/*
	Thread stack sizes
*/

#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PLATFORM_CYGWIN)
	#define THREAD_STACK_SIZE 0
# elif (defined PLATFORM_LINUX) && (defined __i386)
	#define THREAD_STACK_SIZE (2097152/16)
#endif

	

#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)
	void Mutex_Init(AO_MUTEX_T *ao){
		*ao= CreateMutex( NULL /*security attr*/, FALSE /*not locked*/, NULL );
		if (ao == NULL){
			printf("CreateMutex error: %d\n", GetLastError());
       			
		}
}/*end of Mutex_Init*/

void Lock_Mutex( AO_MUTEX_T *ao ) {
    DWORD rc= WaitForSingleObject(*ao,INFINITE);
}
void  Free_Mutex(AO_MUTEX_T *ao){
	 if (!CloseHandle(*ao)){ printf("FreeMutex error: %d\n",GetLastError());}
	ao = NULL;
}
void Unlock_Mutex(AO_MUTEX_T *ao){

if (!ReleaseMutex(*ao))
        printf( "ReleaseMutex: %d\n", GetLastError() );
		}
		
void Create_Thread(AO_THREAD_T *ao, THREAD_RETURN_T(__stdcall *func)( void *),void *data, int priority){
	HANDLE h = (HANDLE)_beginthreadex(NULL,
							THREAD_STACK_SIZE,
							func,
							data,
							0,
							NULL );
	if (h == INVALID_HANDLE_VALUE){
		printf("Error Creating Thread ",GetLastError());
	}
	if (priority != 0){
		int win_priority = (priority == +3) ? THREAD_PRIORITY_TIME_CRITICAL:
							(priority == +2) ? THREAD_PRIORITY_HIGHEST :
							(priority == +1) ? THREAD_PRIORITY_ABOVE_NORMAL :
							(priority == -1) ? THREAD_PRIORITY_BELOW_NORMAL :
							(priority == -2) ? THREAD_PRIORITY_LOWEST :
							THREAD_PRIORITY_IDLE;
	
		if (!SetThreadPriority(h,win_priority)){
			printf("Failed to set thread priority", GetLastError());
		}
	
	}
	*ao = h;
}//end of Create_Thread (win)

void Kill_Thread(AO_THREAD_T *ao){
	if(!TerminatedThread(*ao,0)){
		printf("Thread Failed to terminate", GetLastError());
	}
	*ao = NULL;
}
#else
	/* Linux Stuff*/
	#include <errno.h>
	static void _PT_FAIL( int rc, const char *name, const char *file, uint_t line ) {
    const char *why= (rc==EINVAL) ? "EINVAL" : 
                     (rc==EBUSY) ? "EBUSY" : 
                     (rc==EPERM) ? "EPERM" :
                     (rc==ENOMEM) ? "ENOMEM" :
                     (rc==ESRCH) ? "ESRCH" :
                     //...
                     "";
    fprintf( stderr, "%s %d: %s failed, %d %s\n", file, line, name, rc, why );
    abort();
  }
  
   #define PT_CALL( call ) { int rc= call; if (rc!=0) _PT_FAIL( rc, #call, __FILE__, __LINE__ ); }
  //
  void SIGNAL_INIT( SIGNAL_T *ao ) {
    PT_CALL( pthread_cond_init(ao,NULL /*attr*/) );
    }
  void SIGNAL_FREE( SIGNAL_T *ao ) {
    PT_CALL( pthread_cond_destroy(ao) );
  }
  void SIGNAL_ONE( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_signal(ref) );     // wake up ONE (or no) waiting thread
  }
  void SIGNAL_ALL( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_broadcast(ref) );     // wake up ALL waiting threads
  }

#endif
