
#ifndef REQUEST_H_
#define REQUEST_H_

#include "message.h"
#include "xmlutils.h"

#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

static inline char *which_node (int node_type) {

  switch (node_type) {
    case 0: 
      return "PUBLISHER";
      break;
    case 1:
      return "SUBSCRIBER";
      break;
    case 2:
      return "PUBSUB";
      break;
    case 3:
      return "SEARCHER";
      break;
    case 4:
      return "RESPONDER";
      break;
    case 5:
      return "SEARES";
      break;
    default:
      return NULL;
    }
}

xmlDocPtr createRequestDoc(char *user_id, char *schema_hash, char *nodetype);
int processRequestDoc(xmlDocPtr request_doc, char *user_id, char *schema_hash, char *node_type);
char *encodeRequestDoc(char *user_id, char *schema_hash, char *nodetype, int *request_size);
xmlDocPtr decodeRequestDoc(char *pdu);


#endif
/* End of request.h */
