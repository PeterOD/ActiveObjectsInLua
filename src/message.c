
#include "ao.h"
#include "message.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "lauxlib.h"
#include "lualib.h"
#include "lua.h"

#define MSG "message_"



/* global lua state to create message */
lua_State *MESSAGE = NULL;

  /*mutex for state*/
  AO_MUTEX_T message_mutex;
struct message_t{
  char *id;
  void *data;
  struct message_t *N; /* 'next' pointer */
  struct message_t *P; /* 'previous' pointer */

};

struct message_list_t{
  message start;
  message end;
  int message_count;

};

/* list to hold user data */
mlist init_list(void){
	mlist m;
	m = (mlist)malloc(sizeof(struct message_list_t));
	if(m== NULL){
	  RAISE_ERROR("Unable to assign memory");
		return m;
	}
	m->start=NULL;
	m->end=NULL;
	m->message_count = 0;
	return m;
}

/*Full Userdata, can be extended with metatables 
 * 	http://pgl.yoyo.org/luai/i/lua_newuserdata
 */
message create_message(char *id, void *data){
  message m;

  Lock_Mutex(&message_mutex);
  MESSAGE = luaL_newstate();
  m = (message)lua_newuserdata(MESSAGE,sizeof( struct message_t));
  if(m != NULL){
    lua_pushfstring(MESSAGE,MSG "%s",id);
    m->id = id;
    m->data =data;
  
    m->N =NULL;
    m->P=NULL;
    lua_settable(MESSAGE,-3);
    lua_pop(MESSAGE,1);

    Unlock_Mutex(&message_mutex);
    return m;
  }else{
    luaL_typeerror(MESSAGE,m,MSG);
    Unlock_Mutex(&message_mutex);
    return m;
  }

}

int delete_message(message m, char *id){
  Lock_Mutex(&message_mutex);
  if(m != NULL){
    free(m->N);
    free(m->P);
  }else{
    return 1;
  }

  lua_pushfstring(MESSAGE,MSG "%s",id);
  /*push nil onto the stack */
  lua_pushnil(MESSAGE);
  lua_settable(MESSAGE,-3);
   lua_pop(MESSAGE,1);

 Unlock_Mutex(&message_mutex);
 return 0;

}

/* return a messages' object id */
char *get_id(message m){
	if(m != NULL){
		return m->id;
	}
	RAISE_ERROR("PARAMETER IS NULL");
}

message append(mlist list, message m){
	if((list == NULL) || (m == NULL)){
		RAISE_ERROR("A PARAMETER IS NULL");
		return NULL;
	}
	
	if(list->start == NULL){
		list -> start = m;
		list -> end = m;
	}
	else{
		list->end->N = m;
		m->P = list-> end;
		list->end = m;
	}

	list -> message_count++;
	return m;


}
message append_helper(mlist list, message m){
	Lock_Mutex(&message_mutex);

	return append(list,m);

	Unlock_Mutex(&message_mutex);

}

message find_id(mlist list, char *id){
	message iterator;

	if((list == NULL)||(list -> start == NULL)){
		RAISE_ERROR("A PARAMETER IS NULL");
		return NULL;
	}

	/* search for node from start through to tail */
	for(iterator =list->start; iterator != NULL; iterator=iterator->N){
		if(strcmp(iterator->id,id)==0){
			return iterator;
		}
	}

	/* message not found */
	printf(" message not found returning 'null'");
	return NULL;

}


void remove_message(mlist list, message m){
	message iter;

	/* confirm contents of parameters */
	if((list == NULL)||(list->start == NULL)||(m==NULL)){
		RAISE_ERROR(" A PARAMATER IS NULL");
		return;

	}

	if(list->start == m){
		list->start = m->N;

		if(list->end ==m){
			list->end = m->N;

		}
		else{
			list->start->P = NULL;
		}
		/* free memory */
		free(m);
		list->message_count--;
		return;

	}

	/* search for message from start + 1 to end -1 */
	for(iter = list->start->N; iter != list->end;iter=iter->N){
		if(iter == m){
			m->P->N = m->N;
			m->N->P = m->P;
			free(m);
			return;
		}		

	}

	if(list->end == m){
		list->end = m->P;
		m->P->N = m->N;
		free(m);
		list->message_count--;
		return;

	}


}



void destroy_message_list(mlist list){
	if(list == NULL){
		RAISE_ERROR(" destroy_message_list(): parameter null");
		return;
	}

	while(list->start != NULL){
		remove_message(list,list->start);
	}
	free(list);

}

int get_msg_count(mlist list){
	if(list != NULL){
		return list->message_count;
	}
	RAISE_ERROR("Error -- message count cannot be found (-1) returned");
	return -1;
}
