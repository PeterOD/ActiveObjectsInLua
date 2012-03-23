#include "messagequeue.h"
#include "ao.h"
#include <Stdio.h>
#include <stdlib.h>

#define MESSAGE_QUEUE_LIST_SIZE 10000

struct task_t{
//	void (*tsk)(void*);
	void *data;
};

/* represents node in a list (messages) */
struct message_t{
	
	 struct message_t *N;
	 struct message_t *P;
	unsigned int num_of_msgs;
	task msgs;
	char id;
};

struct message_list_t{
	message start;
	message end;
	unsigned int msg_count;

};

struct messageQueue{
	mlist msgs;	
	list in_use;

	AO_MUTEX_T queue_mutex;

};

struct in_use_node{
	char *id;
	struct in_use_node *next;
	struct in_use_node *prev;

};

struct in_use_list{
	node head;
	node tail;
	int node_count;
};

/*
 * IN_USE_LIST SECTION
 */

/* Initialise an empty list */
list init_list(void){
	list l;
	l = (list)malloc(sizeof(struct in_use_list));
	if(l== NULL){
		return l;
	}
	l->head=NULL;
	l->tail=NULL;
	l->node_count = 0;
	return l;
}
/* create a node for the in use list */
node Create_Node( char *id){

	node n;
	
	n = (node) malloc(sizeof(struct in_use_node));
	if (n == NULL){
		return n;
	}

	n->id = id;
	n->prev = NULL;
	n->next = NULL;
	
	return n;
}

/* Add am id to the in use list note: this is not appended */
node add_id(list l, node n){
	/* check if parameters are null*/
	if(l ==NULL || n == NULL){
		return NULL;
	}

	if(l->head == NULL){
		l->head = n;
		l->tail = n;
	}

	else{/* non - empty list */
		l->tail->next = n;
	 	n->prev = l->tail;
		l->tail = n;	
	
	}

	l->node_count;	
	return n;
}

/* search for an id in the list */
node find_id(list l, char *id){
	/* local node used as iterator node */
	node iterator;

	/* check if list is null or empty */
	if(l == NULL || l->head == NULL){
		RAISE_ERROR("Parameters are null");
		return NULL;
	}
	/* loop to check if is present */
	for(iterator = l->head; iterator != NULL; iterator = iterator->next){
		if(iterator->id == id){
			return iterator;
		}


	}/* end of search loop */

	/* id not found */
	return NULL;
}
/* remove id from list */
void remove_id(list l, node n){
	node iterator;


	/*check if parameters are null or empty */
	if((l == NULL) ||(n == NULL) || (l->head == NULL) ){
		RAISE_ERROR("Parameters are null/Empty");
		return;
	}

	if(l->head == n){
		l->head = n->next;

		if(l->tail == n){
			l->tail = n->next;
		}
		else{
			l->head->prev = NULL;
		}
		free(n);
		l->node_count--;
		return;
	}

	/* id not found in head node therefore search list */
	for(iterator = l->head->next; iterator != l->tail; iterator = iterator -> next){
		if(iterator == n){
			n->prev->next = n->next;
			n->next->prev = n->prev;
			free(n);
			l->node_count--;
			return;
		}
	}
	/* finally! check if node is tail node */
	if(l->tail == n){
		l->tail = n->prev;
		n->prev->next = n->next;
		free(n);
		return;
	}

	return;

}





