
#include <stdio.h>
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "config.h"
#include "request.h"
#include "packedobjectsd.h"

#ifdef DEBUG_MODE

#define dbg(fmtstr, args...)						\
  (printf("libpackedobjectsd" ":%s: " fmtstr "\n", __func__, ##args))
#else
#define dbg(dummy...)
#endif

#ifdef QUIET_MODE

#define alert(dummy...)
#else
#define alert(fmtstr, args...)						\
  (fprintf(stderr, "libpackedobjectsd" ":%s: " fmtstr "\n", __func__, ##args))
#endif

xmlDocPtr create_request(char *user_id, char *schema_hash, char *nodetype)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr pod_node = NULL, message_node = NULL, request_node = NULL;
  
  LIBXML_TEST_VERSION;

  doc = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  pod_node = xmlNewNode(NULL, BAD_CAST "pod");
  xmlDocSetRootElement(doc, pod_node);

  message_node = xmlNewChild(pod_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  request_node = xmlNewChild(message_node, NULL, BAD_CAST "request", BAD_CAST NULL);
    
  /* create nodes to hold data */
  xmlNewChild(request_node, NULL, BAD_CAST "user-id", BAD_CAST user_id);
  xmlNewChild(request_node, NULL, BAD_CAST "schema-hash", BAD_CAST schema_hash);
  xmlNewChild(request_node, NULL, BAD_CAST "node-type", BAD_CAST nodetype);
  //xml_dump_doc(doc);
    
  return doc; /* doc to be freed by calling function */
}

int process_request(xmlDocPtr request_doc, char *user_id, char *schema_hash, char *node_type)
{
  xmlNodePtr cur = xmlDocGetRootElement(request_doc);

  while(cur != NULL) /* traverse to the next XML nodes to read schema-hash and node-type data */
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"user-id")))
       	{
	  while(cur != NULL) /* traverse to the next XML element */
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"user-id"))) {
		xmlChar *key;
		key = xmlNodeListGetString(request_doc, cur->xmlChildrenNode, 1);
		user_id = strcpy(user_id, (char*)key); 
		xmlFree(key);
	      }

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"schema-hash"))) {
		xmlChar *key;
		key = xmlNodeListGetString(request_doc, cur->xmlChildrenNode, 1);
		schema_hash = strcpy(schema_hash, (char*)key);  
		xmlFree(key);
	      }   

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"node-type"))) {
		xmlChar *key;
		key = xmlNodeListGetString(request_doc, cur->xmlChildrenNode, 1);
		node_type = strcpy(node_type, (char*)key);  
		xmlFree(key);
	      }   
	      cur = cur->next;	  
	    }
	  // printf("%s %s %s\n", user_id, schema_hash, node_type);
	  break;
	}
      cur = cur->xmlChildrenNode;
      }

  if((user_id == NULL) ||(schema_hash == NULL) || (node_type == NULL)) {
    return -1;
  }

  return 1;
}

char *encode_request(char *user_id, char *schema_hash, char *nodetype, int *request_size)
{
  xmlDocPtr doc;
  char *pdu = NULL;
  packedobjectsContext *pc;

  pc = init_packedobjects(POD_SCHEMA, 0);
  doc = create_request(user_id, schema_hash, nodetype);

  pdu = packedobjects_encode(pc, doc);
  *request_size =  pc->bytes;
  if (*request_size == -1) {
    alert("Failed to encode with error %d.\n", pc->encode_error);
    return NULL;
  }

  return pdu;
}

xmlDocPtr decode_request(char *pdu)
{
  xmlDocPtr doc = NULL;
  packedobjectsContext *pc;

  pc = init_packedobjects(POD_SCHEMA, 0);
  
  doc = packedobjects_decode(pc, pdu);
  if (pc->decode_error) {
    alert("Failed to decode with error %d.", pc->decode_error);
    return NULL;
  }

  return doc;
}

/* End of request.c */
