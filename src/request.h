
#ifndef REQUEST_H_
#define REQUEST_H_

#include "message.h"

#define MAX_BUFFER_SIZE 100 /* the maximum size for address buffer */
#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

typedef struct {
  char node_type;
  char *schema_hash;  
} Request;

static inline char *which_node (char node_type) {
  if(node_type == 'P') return "publisher";
  else if(node_type == 'S') return "subscriber";
  else return NULL;
}

Request *make_request_object(); 
void free_request_object(Request *req); 
int serialize_request(char *buffer, Request *req); 
int deserialize_request(char *buffer, Request *req);
char *get_broker_detail(char node_type, char *address, int port, char *schema_hash);

#endif
/* End of request.h */
