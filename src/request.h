
#ifndef REQUEST_H_
#define REQUEST_H_

#include "message.h"
#include "xmlutils.h"

#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

static inline char *which_node (char node_type) {
  if(node_type == 'P') return "PUBLISHER";
  else if(node_type == 'S') return "SUBSCRIBER";
  else return NULL;
}

xmlDocPtr create_request(char *user_id, char *schema_hash, char *nodetype);
int process_request(xmlDocPtr request_doc, char *user_id, char *schema_hash, char *node_type);
char *encode_request(char *user_id, char *schema_hash, char node_type, int *request_size);
xmlDocPtr decode_request(char *pdu);


#endif
/* End of request.h */
