#include "ao.h"
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
/* child function i.e. obj.create_message("foo") */
static int ao_create_message(lua_State *L){

	//char *param_id = luaL_checkstring(L,1);
	char *param_id = (char *)lua_isstring(L,1);
	void *param_code;
	
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

