#include "manager.h"
//#include "ao.h"
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
#include <pthread.h>




/*********************************/

pthread_mutex_t state_access = PTHREAD_MUTEX_INITIALIZER;
/* message queue access */
//AO_MUTEX_T queue_access;
pthread_mutex_t queue_access = PTHREAD_MUTEX_INITIALIZER;
/* in_use access */
/*AO_MUTEX_T in_use_mutex;*/
pthread_mutex_t in_use_mutex = PTHREAD_MUTEX_INITIALIZER;
/* mutex to decrement thread_count*/
/*AO_MUTEX_T thread_count_mutex;*/
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;

/* mutex to exit manager (join threads) */
/*AO_MUTEX_T act_mutex;*/
pthread_mutex_t act_mutex = PTHREAD_MUTEX_INITIALIZER;



lua_State *L;


/*
#if defined (PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
	Mutex_Init(&queue_access);
#else
	Mutex_Init(&queue_access);
#endif
*/



/* conditonal to wake thread */

pthread_cond_t wake_thread_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t no_tasks = PTHREAD_COND_INITIALIZER;



int active_count = 0;

int empty_list = FALSE;

list active_tasks;
mlist message_queue;
future f;
/********************************/
#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)

	#include <windows.h>
#elif defined(PLATFORM_LINUX)
	
	#include <unistd.h>
#endif
/*
int get_num_cores(){

#if defined(PLATFORM_WIN32) || defined (PLATFORM_POCKETPC)
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;

#else


	return sysconf(_SC_NPROCESSORS_ONLN);	
#endif

}
*/
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
	/* worker thread loop */
	while(1){
		/*get access to message queue */
		pthread_mutex_lock(&queue_access);
		
		while((in_use_count(active_tasks)== 0)&& (get_msg_count(message_queue) != 0)){
			pthread_cond_wait(&wake_thread_t,&queue_access);
		}
		
		m = pop_message(message_queue);
	
		/* confirm message retrieval */
		if ( m != NULL){
			/*get access to in_use_list */
			pthread_mutex_lock(&in_use_mutex);
			tag = (char*)get_msg_id(m);
			n = Create_Node(tag);
			add_id(active_tasks,n);
			/*release access to in_use_queue*/
			pthread_mutex_unlock(&in_use_mutex);
			
			o = (object)message_code(m);
		}
		else{
				/* unable to process code therefore
					free access to message queue */
					pthread_mutex_unlock(&queue_access);
					pthread_exit(NULL);
			}
		/* m was not null - free access to message queue ( T branch)*/
	
			pthread_mutex_unlock(&queue_access);
		status = lua_resume(L,get_obj_args(o));
		
		reset(o,0);
		
		/* if coroutine was executed successfully */
		if(status == 0){
			/* code has been executed so remove from in_use list */
			pthread_mutex_lock(&in_use_mutex);
			kill_node(n);
			pthread_mutex_unlock(&in_use_mutex);

			
			/*set object status to done*/
			set_obj_status(o,object_thread_done);
			
			/* close shared state of object */
			pthread_mutex_lock(&state_access);
			
			lua_close(L);
			pthread_mutex_unlock(&state_access);
			
			/* descrease the active process count */
			object_count();
		
		}
		
		/* check if thread yielded */
		else if(status == LUA_YIELD){
			if(get_obj_status(o) == object_thread_waiting){
				/* add code and id to future */
				 f = create_future(get_msg_id(m),message_code(m));
				 f = add_to_future(get_msg_id(m),message_code(m),(lua_State *)obj_state(o));
		
				/* destroy node stored on in_use list */
				pthread_mutex_lock(&in_use_mutex);
				kill_node(n);
				pthread_mutex_unlock(&in_use_mutex);
			}
			else{
				/* thread has yielded re-insert into message queue
						and remove from in_use queue
						*/
				pthread_mutex_lock(&queue_access);
					/*check if message is in future */
					if(get_msg_id(m) == fut_id(f)){
						/* do not need future in this case as re-queuing the message */
						kill_future(f);
					}
					
				/* destroying node on in_use list does not destroy message*/
				pthread_mutex_lock(&in_use_mutex);
				kill_node(n);
				pthread_mutex_unlock(&in_use_mutex);				
				append_helper(message_queue,m);
				pthread_mutex_unlock(&queue_access);
			
			}
		}
		else{
			/* check for lua side errors - run time, syntax etc this is bad*/
			pthread_mutex_lock(&in_use_mutex);
			kill_node(n);
			pthread_mutex_unlock(&in_use_mutex);
			if(fut_id(f)){
				kill_future(f);
			}
			pthread_mutex_lock(&queue_access);
			remove_message(message_queue,m);
			pthread_mutex_unlock(&queue_access);
			/*get access to shared lua state */
			pthread_mutex_lock(&state_access);
			
			lua_close((lua_State*)obj_state(o));
			pthread_mutex_unlock(&state_access);
			/* decrement amount of active objects */
			object_count();
		}
	}
	
	return 0;

} /* end of thread_loop function */


int init_manager(int initial_thread_count){
	int startup;
	int thread_count = 0;
	
	pthread_t worker;
	
	/* intialise list and message queue */
	active_tasks = init_in_list();
	message_queue = init_list();
	
	

	for(startup = 0; startup < initial_thread_count; startup++){
		/*Create_Thread(&worker,NULL,thread_loop,2); */
		if(pthread_create(&worker,NULL,thread_loop,NULL)==0){
			thread_count++;
		}
	}
	
	if (thread_count != initial_thread_count){
		/* ERROR: they must be equal to continue */
		return AO_MANAGET_INIT_FAIL;
	}
	return AO_MANAGET_INIT;


}



int create_worker(void){

	pthread_t worker;
	if(pthread_create(&worker,NULL,thread_loop,NULL) !=0){
		return AO_MANAGET_INIT;
	}
	return AO_MANAGET_INIT_FAIL;
}

void object_count(void){
	pthread_mutex_lock(&thread_count_mutex);
	active_count--;
	if(active_count == 0){
		pthread_cond_signal( &no_tasks );
	}
	pthread_mutex_unlock(&thread_count_mutex);
}

void object_count_incr(void){
	pthread_mutex_lock(&thread_count_mutex);
	active_count++;
	pthread_mutex_unlock(&thread_count_mutex);
}
void join_threads_to_exit(void){
	pthread_mutex_lock(&act_mutex);
	int mcnt = get_msg_count(message_queue);
	int icnt = in_use_count(active_tasks);
	if((mcnt == 0) && (icnt == 0)){
		pthread_mutex_lock(&queue_access);
		empty_list = TRUE;
		pthread_cond_broadcast(&wake_thread_t);
		pthread_mutex_unlock(&queue_access);
		
	}
	pthread_mutex_unlock(&act_mutex);
	
}
