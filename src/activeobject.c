#include "ao.h"
#include "future.h"
#include "in_use.h"
#include "message.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "activeobject.h"

#define OBJECT_CREATION_ERROR -1

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


/* active object */
struct active_object{
	
	const char *name;
	void *data;	
	int nargs;
	ao_thread_status_t status;
	future fut;

};
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
	o->fut = create_future(id,data);

	/* have code to open lua standard library 
	 * e.g. function: open_libraries(shared); ??
	 */

	/*
	 * @TODO add child functions for object 
	 * ask does object have to create worker thread */

	/* load data to be executed */
	const char *code = (const char *) o->data;
	int ret_val = luaL_loadstring(shared, code);
	if(ret_val != 0){
		lua_close(shared);
		return  NULL;
	}
	Unlock_Mutex(&state_mutex);
	
	return o;
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

/* registration array for active objects */
static const struct luaL_reg ao_main_functions[] = {
	{"CreateObject",ao_create_object},
	{NULL,NULL}
};

/* registration array for child functions e.g. object.createMessaget */
static const struct luaL_reg ao_child_functions[] = {
	{"createMessage",ao_create_message},
	{NULL,NULL}
};

/* register base libraries with shared state ?? */
static void push_libs(lua_State *L,char *id, lua_CFunction func){
	/* push onto stack "package" */
	lua_getglobal(L,"package");
	/* push "preload" onto first element of stack */
	lua_getfield(L,-1,"preload");
	/* push the func parameter onto stack */
	lua_pushcfunction(L, func);
	/* sets the value at index pos id to value at top of stack
	 *  pops from the stack */
	lua_setfield(L,-2,id);
	/* pop two elements from the stack */
	lua_pop(L,2);		
}

static void load_it(lua_State *L){
	/* lock access to state */
	Lock_Mutex(&state_mutex);
	/* call  c functions in protected mode */
	lua_cpcall(L, luaopen_base, NULL);
	lua_cpcall(L, luaopen_package, NULL);
	push_libs(L,"io",luaopen_io);
	push_libs(L,"os",luaopen_os);
	push_libs(L,"table",luaopen_table);
	push_libs(L,"string",luaopen_string);
	push_libs(L,"math",luaopen_math);
	push_libs(L,"debug",luaopen_debug);

	Unlock_Mutex(&state_mutex);

}
