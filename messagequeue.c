#include "messagequeue.h"
#include "ao.h"
#include <Stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define MESSAGE_QUEUE_LIST_SIZE 10000
#define QUEUE_ACCESS 3
#define QUEUE_ACCESS_FAIL -1


/* global lua state that is used to create messages */
lua_State *MESSAGE = NULL;
AO_MUTEX_T message_state_mutex;
AO_MUTEX_T mlist_access_mutex;

struct task_t{
//	void (*tsk)(void*);
	void *data;
};

/* represents node in a list (messages) */
struct message_t{
	
	 struct message_t *N;
	 struct message_t *P;
	unsigned int num_of_msgs;
 const char  *m_id;
  void *msgs;
	
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


/**********************
 * IN_USE_LIST SECTION
 ***********************/

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

	l->node_count++;	
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
/*************************
 * MESSAGE SECTION
 * ***********************/

/* Initialise a message list for message queue*/
mlist init_message_list(void){
	mlist m;

	m = (mlist)malloc (sizeof(struct message_list_t));

	if(m == NULL){
		RAISE_ERROR(" COULD NOT ASSIGN MEMORY");
		return m;
	}
	m->start = NULL;
	m->end = NULL;
	m->msg_count = 0;

	return m;
}

void message_init(void){
	MESSAGE = luaL_newstate();
	lua_newtable(MESSAGE);
	lua_setglobal(MESSAGE,"message");
}

message create_message(const char *code,const char *mid){
  message m;
  Lock_Mutex(message_state_mutex);

  lua_getglobal(MESSAGE,"message");
  lua_pushstring(MESSAGE,code);
  /* 
     lua_newuserdata; see : http://pgl.yoyo.org/luai/i/lua_newuserdata
  */
  m = (message)lua_newuserdata(MESSAGE,sizeof(struct message_t));
  m-> N = (message)malloc(sizeof(struct message_t));
  if(m->N == NULL){
    RAISE_ERROR("Could not assign memory for struct message_t *N");
  }
  m->P = (message)malloc(sizeof(struct message_t));
   if(m->P == NULL){
    RAISE_ERROR("Could not assign memory for struct message_t *N");
  }
   m->num_of_msgs = 0;
   m->msgs = NULL;
   m->m_id = mid;
   
   lus_settable(MESSAGE,-3);
   lua_pop(MESSAGE,1);

   Unlock_Mutex(message_state_mutex);
   return m;
}

message get_message(const char *mid){
  message m;

  Lock_Mutex(message_state_mutex);

  /* search for the message on the stack */
  lua_getglobal(MESSAGE,"message");
  /* -1 index is for top of stack? */
  lua_getfield(MESSAGE,-1,mid);

  if((lua_type(MESSAGE, -1 )) == LUA_TUSERDATA  ){
    m = (message)lua_touserdata(MESSAGE,-1);
 }
  else{
    m = NULL;
  }

  lua_pop(MESSAGE,2);
  Unlock_Mutex(message_state_mutex);

  return m;
  
  
}
const char *get_msg_id(message m){

  return m->m_id;
}

/**************************************************************
 * MESSAGE LIST SECTION (list that contains messages for queue)
 **************************************************************/

mlist init_mlist(void){
  mlist mlst;

  mlst = (mlist)malloc(sizeof(struct message_list_t));
  if(mlst == NULL){
    return mlst;
  }
  mlst -> start = NULL;
  mlst -> end = NULL;
  mlst -> msg_count = 0;
  return mlst;

}

message mlist_append(mlist m, message msg){
  /* 
     confirm if mutex lock should be part of message_list_t struct
     or 'global' lock
     note: on 25/3/12 'global' was chosen before confirmation 
  */
  
  /* get access to list */
	Lock_Mutex(mlist_access_mutex);

	return append(m,msg);

	Unlock_Mutex(mlist_access_mutex);



}

message append (mlist m, message msg){
  /* will have to be modified as messages are
     lua_userdata?? */
  /* check if parameters are null */
  if(m == NULL || msg == NULL){
    return NULL;
  }

  if(m -> start == NULL){
    m->start = msg;
    m->end = msg;
  }
  else{
    m->end->N = msg;
    msg -> P = m -> end;
    m->end = msg;
  }

  m->msg_count++;
  return msg;

}



