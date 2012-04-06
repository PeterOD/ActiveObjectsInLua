#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H
#include "activeobject.h"
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
	Function:	create_obj_worker
	
	Parameters:	L - pointer to lua state
				
	Returns:	integer denoting number of elements from stack
*/
static int create_obj_worker(lua_State *L);

/*
	Function:	enqueue_object_message
	
	Parameters:	o - active object
	
	This function enqueus an object's message onto the message queue
				
	
*/


void enqueue_object_message(object o);


lua_State *obj_state(object o);

void save_values(lua_State *from, lua_State *to);

object return_object(lua_State *L);
/**********************
 * 	C API
 **********************/
static int ao_create_object(lua_State *L);

object create_object(const char *id, void *data);
int get_obj_args(object o);
void reset(object o, int rst);


/* child functions i.e. object.create_message("foo") */
static int ao_create_message(lua_State *L);

static int ao_send_msg(lua_State *L);




#endif
