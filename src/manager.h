#ifndef MANAGER_H
#define MANAGER_H
#include "ao.h"
#include "in_use.h"




#define AO_MANAGET_INIT 0
#define AO_MANAGET_INIT_FAIL -1

/* 
	Function:	init_manager
	
	Parameters:	initial_thread_count - the number of threads to create
	
	returns:	the number of threads created
*/
int init_manager(int initial_thread_count);
/* 
	Function:	create_worker
	
	Parameters:	void
	
	returns:	a worker thread
*/

int create_worker(void);
/* 
	Function:	object_count
	
	Parameters:	void
	
	returns:	decrements number of messages
*/

void object_count(void);
/* 
	Function:	object_count_incr
	
	Parameters:	void
	
	returns:	increments number of messages
*/

void object_count_incr(void);


#endif