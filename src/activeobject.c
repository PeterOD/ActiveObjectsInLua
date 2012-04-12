#include "ao.h"
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

#define OBJECT_CREATION_ERROR -1
#define IGNORE_TOP_STK_INDEX 2






/* message mutex*/
AO_MUTEX_T message_mutex;

/* state mutex */
AO_MUTEX_T state_mutex;
/* shared state */
lua_State *shared  = NULL;

/* in_use mutex lock */
AO_MUTEX_T list_mutex;

/*global in_use list to hold object ids */
list active;

/* global message queue that holds tasks */
mlist message_queue;

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


/* active object */
struct active_object{
	
	const char *name;
	void *data;	
	int nargs;
	/*AO_THREAD_T obj_thread;*/
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
	{NULL,NULL}
};

/* registration array for child functions e.g. object.createMessaget */
static const struct luaL_reg ao_child_functions[] = {
	{"createMessage",ao_create_message},
	{"NewKernelWorker",ao_kernel_worker},
	{"ExecuteMessage",ao_exec},
	{NULL,NULL}
};


/******************************************/

object create_object(const char *id, void *data){

	object o;

	/* get access to shared lua state */
	Lock_Mutex(&state_mutex);
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
	Unlock_Mutex(&state_mutex);
	
	return o;
}

int get_obj_args(object o){
	return o->nargs;

}
lua_State* obj_state(object o){
	if(o != NULL){
		return o->ostate;
	}

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

	//char *param_id = luaL_checkstring(L,1);
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
	/* @TODO check if queue is inited; yes append, no create then append */
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
}

static int create_obj_worker(lua_State *L){
	if(create_worker() != AO_MANAGET_INIT){
		lua_pushnil(L);
		lua_pushstring(L,"Unable to create kernel thread");
		return 2; /* no of items returned from stack */
	}
	lua_pushboolean(L,TRUE);
	return 1;

}

void enqueue_object_message(object o){
	Lock_Mutex(&message_mutex);
	
	append_helper(message_queue,o->m);
	Unlock_Mutex(&message_mutex);
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
	Lock_Mutex(&state_mutex);
	lua_getfield(L,LUA_REGISTRYINDEX, "__self__");
	o = (object)lua_touserdata(L,-1); /* (-1) refers to top of stack */
	lua_pop(L,1);
	Unlock_Mutex(&state_mutex);
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
	
	const char *code = luaL_checkstring(shared,index);
	lua_gettable(shared, LUA_REGISTRYINDEX);
	lua_State * temp = get_fut(f);
	base = lua_gettop(temp); 
	index = index +1;
	Lock_Mutex(&state_mutex);
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
	Unlock_Mutex(&state_mutex);
	lua_pushboolean(shared,0);
	lua_pushstring(shared,lua_tostring(get_fut(f),-1));
	lua_pop(get_fut(f),2);
	return 2;
}

static int ao_exec(lua_State *L){
	
	lua_gettable(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);
	Lock_Mutex(&message_mutex);
	message m = peek(message_queue);
	Lock_Mutex(&list_mutex);
	node n = find_in_id(active,get_msg_id(m));
	Unlock_Mutex(&list_mutex);
	if(in_id(n) == get_msg_id(m)){
		m = peek_next(m);
	}
	
	
	return do_stuff(L,m,1);
}

int luaopen_activeobject(lua_State *L){
	
	luaL_register(L,"activeobject",ao_main_functions);
	
	int initial = get_num_cores();
	/* initialise the manager */
	init_manager(initial);

	/* create in_use list and message queue */	
	active = init_in_list();
	message_queue = init_list();
	
	return 0;
}



