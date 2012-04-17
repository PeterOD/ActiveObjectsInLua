#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H

#include "activeobject.h"
#include "lua.h"
#include "message.h"
typedef struct active_object *object;

typedef enum{
	object_thread_idle = 0,
	object_thread_waiting = 1,
	object_thread_running = 2,
	object_thread_done = 3
	
	}ao_thread_status_t;


/***********************
 * BACKGROUND
 ***********************/
/*
	Function:	create_object
	
	Parameters:	id - object id
				code - code to be executed
	Returns:	a new 'active' object
*/
 
object create_object(const char *id, void *code); 

/*
	Function:	set_obj_status
	
	Parameters:	o - object 
				stat - status constant of object

*/
void set_obj_status(object o, int stat);

/*
	Function:	get_obj_status
	
	Parameters:	o - object
				
	Returns:	a status constant
*/
int get_obj_status(object o);



/*
	Function:	enqueue_object_message
	
	Parameters:	o - active object
	
	This function enqueus an object's message onto the message queue
				
	
*/


void enqueue_object_message(object o);
static int ao_create_object(lua_State *L);

object create_object(const char *id, void *data);
int get_obj_args(object o);
void reset(object o, int rst);
/*static int ao_send_message(lua_State *L); */



/* child functions i.e. object.create_message("foo") */
static int ao_create_message(lua_State *L);


lua_State *obj_state(object o);

void save_values(lua_State *from, lua_State *to);

object return_object(lua_State *L);
static int ao_kernel_worker(lua_State *L);
static int ao_queue_size(lua_State *L);

static int function_ready(lua_State *L, message m);
static int do_stuff(lua_State *shared, message m, int index);
static int ao_exec(lua_State *L);



/*static int ao_send_message(lua_State *L); */







static int ao_exit(lua_State *L);


#endif
