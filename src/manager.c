#include "manager.h"
#include "ao.h"
#include "message.h"
#include "in_use.h"
#include "activeobject.h"
#include "future.h"
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
AO_MUTEX_T state_access;
/* message queue access */
AO_MUTEX_T queue_access;
/* in_use access */
AO_MUTEX_T in_use_mutex;

/* mutex to decrement thread_count*/
AO_MUTEX_T thread_count_mutex;
lua_State *L;


/*
#if defined (PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
	Mutex_Init(&queue_access);
#else
	Mutex_Init(&queue_access);
#endif
*/



/* conditonal to wake thread */
SIGNAL_T wake_thread;

#if defined (PLATFORM_LINUX)
	/*conditional to allow the thread count of ZERO to be broadcast */
	SIGNAL_T *no_tasks; 
#else
	//CONDITION_VARIABLE no_tasks;
#endif

int active_count = 0;

int empty_list = 0;

list active_tasks;
mlist message_queue;
future f;
/********************************/
#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)

	#include <windows.h>
#elif defined(PLATFORM_LINUX)
	
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
    /* message to be used as aux in order to execute thread code */
	message m;
	object o;
	node n;
	char* tag;
	int status;
	int kill_worker;
	/* worker thread loop */
	while(1){
		/*get access to message queue */
		Lock_Mutex(&queue_access);
		
		m = pop_message(message_queue);
		/* confirm message retrieval */
		if ( m != NULL){
			/*get access to in_use_list */
			Lock_Mutex(&in_use_mutex);
			tag = (char*)get_msg_id(m);
			n = Create_Node(tag);
			add_id(active_tasks,n);
			/*release access to in_use_queue*/
			Unlock_Mutex(&in_use_mutex);
			
			o = (object)message_code(m);
		}
		else{
				/* unable to process code therefore
					free access to message queue */
					Unlock_Mutex(&queue_access);
			}
		/* m was not null - free access to message queue ( T branch)*/
		Unlock_Mutex(&queue_access);
		
		status = lua_resume(L,get_obj_args(o));
		
		reset(o,0);
		
		/* if coroutine was executed successfully */
		if(status == 0){
			/* code has been executed so remove from in_use list */
			kill_node(n);
			
			/*set object status to done*/
			set_obj_status(o,object_thread_done);
			
			/* close shared state of object */
			Lock_Mutex(&state_access);
			
			lua_close(L);
			Unlock_Mutex(&state_access);
			
			/* descrease the active process count */
			object_count();
		
		}
		
		/* check if thread yielded */
		else if(status = LUA_YIELD){
			if(get_obj_status(o) == object_thread_waiting){
				/* add code and id to future */
				 f = create_future(get_msg_id(m),message_code(m));
		#if defined (PLATFORM_LINUX)		
			SIGNAL_ONE(&no_tasks);
		#else
			/* win32 */

		#endif
				/* destroy node stored on in_use list */
				kill_node(n);
			}
			else{
				/* thread has yielded re-insert into message queue
						and remove from in_use queue
						*/
				Lock_Mutex(&queue_access);
					/*check if message is in future */
					if(get_msg_id(m) == fut_id(f)){
						/* do not need future in this case as re-queuing the message */
						kill_future(f);
					}
					
				/* destroying node on in_use list does not destroy message*/
				kill_node(n);
				
				append_helper(message_queue,m);
				Unlock_Mutex(&queue_access);
			
			}
		}
		else{
			/* check for lua side errors - run time, syntax etc this is bad*/
			kill_node(n);
			if(fut_id){
				kill_future(f);
			}
			remove_message(message_queue,m);
			/*get access to shared lua state */
			Lock_Mutex(&state_access);
			
			lua_close((lua_State*)obj_state(o));
			Unlock_Mutex(&state_access);
			object_count();
		}
	}
	
	return 0;

} /* end of thread_loop function */


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

void object_count(void){
	Lock_Mutex(&thread_count_mutex);
	active_count--;
	if(active_count == 0){
	#if defined (PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
		/*WakeConditionVariable(&no_tasks); */
	#else
		SIGNAL_ONE(*no_tasks);
	#endif
	}
	Unlock_Mutex(&thread_count_mutex);
}

void object_count_incr(void){
	Lock_Mutex(&thread_count_mutex);
	active_count++;
	Unlock_Mutex(&thread_count_mutex);
}
