#include "ao.h"
#include "message.h"
#include "in_use.h"
#include <stdlib.h>
#include <stdio.h>

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

/* Initialise an empty list */
list init_in_list(void){
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

/* Add an id to the in use list  */
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

	l->node_count++;	
	return n;
}

/* search for an id in the list */
node find_in_id(list l, char *id){
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

void destroy_list(list l){
	if(l == NULL){
	RAISE_ERROR("Attempted to destroy empty list");
		return;
	}

	while(l->head !=NULL){
		remove_id(l,l->head);
	}
	free(l);


}

void kill_node(node n){
	if(n != NULL){
		free(n);
		n= NULL;
	}
	RAISE_ERROR("Could not delete node ");
}

int in_use_count(list l){
	if(l != NULL){
		return l->node_count;
	}
	return -1;
}
char *in_id(node n){
	if(n != NULL){
		return n->id;
	}
	RAISE_ERROR("NODE EMPTY -- could not find id");
}
