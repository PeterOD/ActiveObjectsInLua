#include "future.h"
#include "message.h"
#include "ao.h"
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
	const char *id;
	bool_t done;
	void *ret_value;
	
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
	return f;

}

future create_future(const char *id, void *code){
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
future add_to_future(future f, const char* id, void *code){
	/* check if parameters are null */
	if((f == NULL) || (id == NULL) || (code = NULL)){
		RAISE_ERROR(" A Parameter is NULL ");
		return f;
	}
	
	f->id = id;
	f->ret_value =code;
	f->done = check_status(f);
	return f;

}