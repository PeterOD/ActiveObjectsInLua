#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H

typedef struct active_object *object;


/***********************
 * BACKGROUND
 ***********************/

object create_object(const char *id, void *code); 



/**********************
 * 	C API
 **********************/
static int ao_create_object(lua_State *L);


/* child functions i.e. object.create_message("foo") */
static int ao_create_message(lua_State *L);

static int ao_send_msg(lua_State *L);




#endif
