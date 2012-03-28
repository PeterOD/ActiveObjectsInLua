#ifndef MESSAGE_H
#define MESSAGE_H

#define RAISE_ERROR(...) fprintf(stderr,"line %d - ", __LINE__); fprintf(stderr, __VA_ARGS__); fprintf (stderr,"\n")

/*
 This file contains the operations that run in the background
 on the linked list that represent the messages

 structure:

 { {id, { message}}->{id,{message}} }
*/

typedef struct message_t *message;
typedef struct message_list_t *mlist;

/*
  Function:  init_list

  Parameters: none

  returns: mlist

  This function initialies an empty list
*/
mlist init_list(void);

/*
 
	Function:  create_message

	Parameters:	id - the id of the messages' object
			data - message data
	
	returns:	message

	This function creates a new message and pushes it
	onto stack
*/
message create_message(char *id, void *data);

/*
 
   	Function: delete_message

	Parameters:	m - message to delete
			id- id of messages' object

	returns:	int

*/
int delete_message(message m, char *id);

/*
 
   	Function:       get_id

	Parameters:	m - message to delete
			

	returns:	* char

*/
char *get_id(message m);

/*
 
   	Function:       append

	Parameters:	list - message list
			m - message append
			

	returns:	message

*/
message append(mlist list, message m);

/*
 
   	Function:       append_helper

	Parameters:	list - message list
			m - message append
			

	returns:	message

	This function is a helper to the append function
	it allows for exclusive access to the queue via a lock

*/

message append_helper(mlist list, message);

/*
 
   	Function:       find_id

	Parameters:	list - message list
			id - id to 
			

	returns:	message

	this function searches for a message via an id an returns
	it if found

*/

message find_id(mlist list, char *id);

/*
 
   	Function:       remove_message

	Parameters:	list - message list
			m - message to be removed
			

	returns:	void

	this function removes a message from the message list

*/
void remove_message(mlist list, message m);




/*
 
   	Function:       destroy_message_list

	Parameters:	list - message list
			
			

	returns:	void

	this function destroys a message list

*/
void destroy_message_list(mlist list);
/*
 
   	Function:       get_msg_count

	Parameters:	list - message list
			
			

	returns:	message_count



*/


int get_msg_count(mlist list);

#endif
