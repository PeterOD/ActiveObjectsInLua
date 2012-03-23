#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H


typedef struct task_t *task;
typedef struct message_t *message;
typedef struct message_list_t *mlist;
typedef struct messageQueue *queue;
typedef struct in_use_list *list;
typedef struct in_use_node *node;

#define RAISE_ERROR(...) fprintf(stderr,"line %d - ", __LINE__); fprintf(stderr, __VA_ARGS__); fprintf (stderr,"\n")


/*
 * 	Function:	init_list
 * 	
 * 	Parameter:	void
 *
 * 	Returns:	list
 *
 * 	This functions initialises an empty list
 */
list init_list(void);

/*
 * 	Function:	Create_Node
 *
 * 	Parameters:	char id
 *
 * 	Returns:	Node
 *
 * 	This functions creates a new node for the in_use_list
 */
node Create_Node( char *id);

/*
 * 	Function:	add_id
 *
 * 	Parameters:	list l, node n
 *
 * 	Returns:	Node
 *
 * 	This functions adds a node which contains an object id to the list
 */
node add_id(list l, node n);


/*
 * 	Function:	add_id
 *
 * 	Parameters:	 list l, char id
 *
 * 	Returns:	void
 *
 * 	This function searches for an id in the list
 */
node find_id(list l, char *id);

/*
 * 	Function:	remove_id
 *
 * 	Parameters:	list l, node n
 *
 * 	returns:	void
 *
 * 	This function removes a node from the in_use_list. This is done
 * 	because the object with the id being removed is no longer in use
 */

void remove_id(list l, node n);


#endif
