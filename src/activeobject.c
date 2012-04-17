//#include "ao.h"
#include "future.h"
#include "in_use.h"
#include "message.h"
#include "manager.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "activeobject.h"
#include <pthread.h>


#define OBJECT_CREATION_ERROR -1
#define IGNORE_TOP_STK_INDEX 2

#ifdef _WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif




/* message mutex*/
pthread_mutex_t a_message_mutex = PTHREAD_MUTEX_INITIALIZER;

/* state mutex */
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
/* shared state */
lua_State *shared  = NULL;

/* in_use mutex lock */
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

/*global in_use list to hold object ids */
list active;

/* global message queue that holds tasks */
mlist message_queue;

int get_num_cores() 
{
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif MACOS
    int nm[2];
    size_t len = 4;
    uint32_t count;
 
    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
 
    if(count < 1) {
    nm[1] = HW_NCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}


/* active object */
struct active_object{
	
	const char *name;
	void *data;	
	int nargs;
	/*pthread_t obj_thread;*/
	 ao_thread_status_t status;
	future fut;
	message m;
	lua_State *ostate;

};


/*******************************************
*	Registration
*******************************************/

/* registration array for active objects */
static const struct luaL_reg ao_main_functions[] = {
	{"CreateObject",ao_create_object},
	{"NewKernelWorker",ao_kernel_worker},
	{"QueueSize",ao_queue_size},
	{"Close",ao_exit},
	{NULL,NULL}
};

/* registration array for child functions e.g. object.createMessaget */
static const struct luaL_reg ao_child_functions[] = {
	{"createMessage",ao_create_message},
	{"NewKernelWorker",ao_kernel_worker},
	{"ExecuteMessage",ao_exec},
	{"Close",ao_exit},
	{NULL,NULL}
};


/******************************************/

object create_object(const char *id, void *data){

	object o;

	/* get access to shared lua state */
	pthread_mutex_lock(&state_mutex);
	shared = luaL_newstate();

	/* store the object in shared state ?? */
	o = (object) lua_newuserdata(shared, sizeof(struct active_object));
	lua_setfield(shared, LUA_REGISTRYINDEX, "object");

	o->name = id;
	o-> data = data;
	o-> nargs = 0;
	o->status = object_thread_idle;
	o->fut = create_future((char*)id,data);
	o->m = create_message(id,data);
	o->ostate = shared;

	/* load base libraries */
	luaL_openlibs(o->ostate);

	luaL_register(shared,"activeobject",ao_child_functions);	

	/* load data to be executed */
	const char *code = (const char *) o->data;
	int ret_val = luaL_loadstring(o->ostate, code);
	if(ret_val != 0){
		lua_close(o->ostate);
		return  NULL;
	}
	pthread_mutex_unlock(&state_mutex);
	
	return o;
}

int get_obj_args(object o){
	return o->nargs;

}
lua_State* obj_state(object o){
	if(o != NULL){
		return o->ostate;
	}
	return o->ostate;

}
void reset(object o, int rst){
	if(o != NULL){
		o->nargs = rst;
	}
	RAISE_ERROR("null object");
	return;
}
static int ao_create_object(lua_State *L){
	
	const char *param_id = luaL_checkstring(L,1);
	char *param_code =(char *)lua_isstring(L,2);
	
	object ao;
	
	ao = create_object(param_id,param_code);
	/* error checking */
	if(ao == NULL){
		lua_pushnil(L);
		lua_pushstring(L,"Error creating object");
		return OBJECT_CREATION_ERROR;		
	}

	lua_pushboolean(L,TRUE);
	return 1;
	
}

/* child function i.e. obj.create_message("foo") */
static int ao_create_message(lua_State *L){


	char *param_id = (char *)lua_isstring(L,1);
	char *param_code =(char *)lua_isstring(L,2);
	
	message new_msg;
	
	new_msg = create_message(param_id,param_code);
	
	/* error checking */
	if(new_msg == NULL){
		lua_pushnil(L);
		lua_pushstring(L,"Error loading code to be executed");
		return 2;
	}
	/* add message to message queue*/

	message_queue = init_list();
	append_helper(message_queue,new_msg);
	
	/* message created */
	lua_pushboolean(L,TRUE);
	return 1;

}

void set_obj_status(object o, int stat){
	o->status = stat;
}
int get_obj_status(object o){
	if(o != NULL){
		return o->status;
	}
	RAISE_ERROR("cannot get status of object");
	return 0;
}



void enqueue_object_message(object o){
	pthread_mutex_lock(&a_message_mutex);
	
	append_helper(message_queue,o->m);
	pthread_mutex_unlock(&a_message_mutex);
}

/* move values to lua stack in future structure */
void save_values(lua_State *from, lua_State *to){
	int idx;
	/*size of stack */
	int num_args = lua_gettop(from);
	for(idx = IGNORE_TOP_STK_INDEX; idx<= num_args; idx++){
			lua_pushstring(to,lua_tostring(from,idx));

		}	
}

/* return the current object contained in the shared state */
object return_object(lua_State *L){
	object o;
	/* allow exclusive access to stack functions */
	pthread_mutex_lock(&state_mutex);
	lua_getfield(L,LUA_REGISTRYINDEX, "__self__");
	o = (object)lua_touserdata(L,-1); /* (-1) refers to top of stack */
	lua_pop(L,1);
	pthread_mutex_unlock(&state_mutex);
	return o;
}

static int ao_kernel_worker(lua_State *L){
	if(create_worker() == AO_MANAGET_INIT){
		lua_pushboolean(L,TRUE);
		return 1;
	}
	lua_pushnil(L);
	lua_pushstring(L,"Error creating kernel worker");
	return 2;

}

static int ao_queue_size(lua_State *L){
	int n = get_msg_count(message_queue);
	return lua_tonumber(L,n);
}

static int function_ready(lua_State *L, message m){
	if(m == NULL){
		RAISE_ERROR("Cannot execute code -- message is NULL:  %s",get_msg_id(m));
	}
	
	lua_pushlightuserdata(L,message_code(m));
	/* push code onto LUA_REGISTRYINDEX see: http://www.lua.org/pil/27.3.1.html */
	lua_gettable(L,LUA_REGISTRYINDEX);
	/* push id onto stack */
	lua_pushstring(L,get_msg_id(m));
	lua_gettable(L,-2);
	if(! lua_isfunction(L,-1)){
		int status;
		/* remove from top of stack */
		lua_pop(L,1);
		status = luaL_loadbuffer(L,message_code(m),strlen(message_code(m)),message_code(m));
		if(status != 0){
			
			lua_remove(L,-2);
			return status;
		}
		
		lua_pushliteral(L,"active_object_env");
		lua_gettable(L,LUA_REGISTRYINDEX);
		if (m == NULL){
			lua_pushliteral(L,"message_code");
		}
		else{
			lua_pushlightuserdata(L,message_code(m));
		}
		lua_gettable(L,-2);
		if(!lua_isnil(L,-1)){
			/* http://pgl.yoyo.org/luai/i/lua_setfenv */
			lua_setfenv(L,-3);
			lua_pop(L,-1);
		}
		else{
			/* pop 2 items from table */
			lua_pop(L,2);
		}
		
	}
	lua_remove(L,-2);
	return 0;
	
}

static int do_stuff(lua_State *shared, message m, int index){
	int base;
	future f = create_future(get_msg_id(m),message_code(m));
	
	
	lua_gettable(shared, LUA_REGISTRYINDEX);
	lua_State * temp = get_fut(f);
	base = lua_gettop(temp); 
	index = index +1;
	pthread_mutex_lock(&state_mutex);
	if(function_ready(temp,m)== 0){
		int arg_num = lua_gettop(shared);
		add_to_future(get_msg_id(m),message_code(m),temp);
		arg_num = arg_num-index+1;
		if(lua_pcall(temp,arg_num,LUA_MULTRET,base)== 0){
			int ret_val = lua_gettop(get_fut(f));
			lua_pushboolean(shared,1);
			lua_xmove(get_fut(f),shared,ret_val);
			/* return this number from stack */
			ret_val = ret_val-base+1;
			lua_pop(shared,ret_val);
			return 1+(ret_val-base);
		}
	
	}
	pthread_mutex_unlock(&state_mutex);
	lua_pushboolean(shared,0);
	lua_pushstring(shared,lua_tostring(get_fut(f),-1));
	lua_pop(get_fut(f),2);
	return 2;
}

static int ao_exec(lua_State *L){
	
	lua_gettable(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);
	pthread_mutex_lock(&a_message_mutex);
	message m = peek(message_queue);
	pthread_mutex_lock(&list_mutex);
	node n = find_in_id(active,get_msg_id(m));
	pthread_mutex_unlock(&list_mutex);
	if(in_id(n) == get_msg_id(m)){
		m = peek_next(m);
	}
	
	
	return do_stuff(L,m,1);
}

LUALIB_API int luaopen_activeobject(lua_State *L){
	
	luaL_register(L,"activeobject",ao_main_functions);
	
	int initial = get_num_cores();
	/* initialise the manager */
	init_manager(initial);

	/* create in_use list and message queue */	
	active = init_in_list();
	message_queue = init_list();
	
	return 0;
}

static int ao_exit(lua_State *L){
	join_threads_to_exit();
	return 0;
}


