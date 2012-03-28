#ifndef IN_USE_H
#define IN_USE_H

typedef struct in_use_node *node;
typedef struct in_use_list *list;



/*
 * 	Function:	init_list
 * 	
 * 	Parameter:	void
 *
 * 	Returns:	list
 *
 * 	This functions initialises an empty list
 */
list init_in_list(void);

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
node find_in_id(list l, char *id);

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

/*
 * 	Function: destroy_list
 *
 * 	Parameter: list l
 *
 * 	Returns: void
 *
 * 	This function destroys the list
 */ 
void destroy_list(list l);

#endif
