#include "future.h"
#include "message.h"
#include "ao.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
//#include "activeobject.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/*
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
*/

struct future_t{
	 char *id;
	bool_t done;
	void *ret_value;
	lua_State *fut;
	
};

future init_future(void){

	future f;
	f = (future) malloc(sizeof(struct future_t));
	if (f == NULL){
		RAISE_ERROR("Unable to allocate memory for future ");
		return f;
	}
	f->id = NULL;
	f->done = FALSE;
	f->ret_value = NULL;
	f->fut = luaL_newstate();
	return f;

}

future create_future(char *id, void *code){
	future f;
	f = init_future();
	f->id = id;
	f -> ret_value = code;
	return f;

}

bool_t is_not_done(future f){
	if (f != NULL){
		return f->done = FALSE;
	}
}

/* sets bool_t to TRUE */
bool_t completed(future f){
	if(f != NULL){
		return f->done = TRUE;
	}
}

bool_t check_status(future f){
	bool_t check_it;
	if(check_it = is_not_done(f)){
		return check_it;
	}else if(check_it = completed(f)){
		return check_it;
	}
}
future add_to_future(  char* id, void *code,lua_State *L){
	future f;
	/* check if parameters are null */
	if((f == NULL) || (id == NULL) || (code = NULL)){
		RAISE_ERROR(" A Parameter is NULL ");
		return f;
	}
	
	f->id = id;
	f->ret_value =code;
	f->done = check_status(f);
	
	/* arguments is number of items on stack */
	int arguments = lua_tonumber(L, -1);
	/* pop that item from stack */
	lua_pop(L,1);
	if(arguments){
		lua_xmove(L,f->fut,arguments);
	}
	return f;

}

char *fut_id(future f){
	if(f != NULL){
		return f->id;
	}
} 

void kill_future(future f){
	if (f == NULL){
		RAISE_ERROR(" null future struct passed into function");
		return;
	}
	/*close the future's lua state pointer */
	lua_close(f->fut);
	free(f);

}
lua_State *get_fut(future f){
	return f->fut;
}