#ifndef FUTURE_H
#define FUTURE_H
//#include "ao.h"
#include "message.h"
#include <lua.h>
typedef struct future_t *future;
typedef void * THREAD_RETURN_T;
typedef int bool_t;
#ifndef FALSE
# define FALSE 0
# define TRUE 1
#endif


/*meta table of future*/
/*static const char future_tab[] = "__futureTab";*/

/*
	Function:	init_future
	
	Paramters:	void

	Returns:	initialise (empty) future
*/
future init_future(void);

/*
	Function:	new_future
	
	Paramters:	id - message id
				code - code
				

	Returns:	creates a new future
	
	
*/
future create_future(char *id, void *code);

/*
	Function:	is_done
	
	Paramters:	f - the future struct
				

	Returns:	done boolean
	
	
*/
bool_t is_not_done(future f);

/*
	Function:	completed
	
	Paramters:	f - the future struct
				

	Returns:	done boolean
	
	
*/
bool_t completed(future f);

/*
	Function:	check_status
	
	Paramters:	f - the future struct
				

	Returns:	done boolean
	
	
*/

bool_t check_status(future f);
/*
	Function:	add_to_future
	
	Paramters:	f - the future struct
				id - id of message
				code - code to be completer
				

	Returns:	future
	
	
*/
future add_to_future(  char* id, void *code, lua_State *L);

char *fut_id(future f);

void kill_future(future f);

lua_State *get_fut(future f);


#endif
