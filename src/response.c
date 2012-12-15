
#include <stdio.h>
#include <stdlib.h>  /* for free() */
#include <string.h> /* for  memcpy() & strlen() */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "config.h"
#include "response.h"
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

xmlDocPtr create_response(char *broker_address, char *port_in, char *port_out, char *process_id)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr pod_node = NULL, message_node = NULL, response_node = NULL;
  
  LIBXML_TEST_VERSION;

  doc = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  pod_node = xmlNewNode(NULL, BAD_CAST "pod");
  xmlDocSetRootElement(doc, pod_node);

  message_node = xmlNewChild(pod_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  response_node = xmlNewChild(message_node, NULL, BAD_CAST "response", BAD_CAST NULL);
    
  /* create nodes to hold data */
  xmlNewChild(response_node, NULL, BAD_CAST "broker-address", BAD_CAST broker_address);
  xmlNewChild(response_node, NULL, BAD_CAST "port-in", BAD_CAST port_in);
  xmlNewChild(response_node, NULL, BAD_CAST "port-out", BAD_CAST port_out);
  xmlNewChild(response_node, NULL, BAD_CAST "process-id", BAD_CAST process_id);
  // xml_dump_doc(doc);
    
  return doc; /* doc to be freed by calling function */
}

char *encode_response(char *broker_address, int port_in, int port_out, int process_id, int *response_size)
{
  xmlDocPtr doc;
  char *pdu = NULL;
  char portin[20];
  char portout[20];
  char processid[20];
  packedobjectsContext *pc;

  sprintf(portin, "%d", port_in);
  sprintf(portout, "%d", port_out);
  sprintf(processid, "%d", process_id);

  pc = init_packedobjects("../schema/packedobjectsd.xsd");
  doc = create_response(broker_address, portin, portout, processid);

  pdu = packedobjects_encode(pc, doc);
  *response_size =  pc->bytes;
  if (*response_size == -1) {
    alert("Failed to encode with error %d.\n", pc->encode_error);
    return NULL;
  }

  return pdu;
}

int process_response(xmlDocPtr response_doc, char *broker_address, int *port_in, int *port_out, int *process_id)
{
  xmlNodePtr cur = xmlDocGetRootElement(response_doc);

  while(cur != NULL) /* traverse to the next XML nodes to read broker address and port numbers data */
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"broker-address")))
       	{
	  while(cur != NULL) /* traverse to the next XML element */
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"broker-address"))) {
		xmlChar *key;
		key = xmlNodeListGetString(response_doc, cur->xmlChildrenNode, 1);
		broker_address = strcpy(broker_address, (char*)key); 
		xmlFree(key);
	      }

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"port-in"))) {
		xmlChar *key;
		key = xmlNodeListGetString(response_doc, cur->xmlChildrenNode, 1);
		*port_in = atoi((char*)key);  
		xmlFree(key);
	      }   

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"port-out"))) {
		xmlChar *key;
		key = xmlNodeListGetString(response_doc, cur->xmlChildrenNode, 1);
		*port_out = atoi((char*)key);  
		xmlFree(key);
	      }   
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"process-id"))) {
		xmlChar *key;
		key = xmlNodeListGetString(response_doc, cur->xmlChildrenNode, 1);
		*process_id = atoi((char*)key);  
		xmlFree(key);
	      }
	      cur = cur->next;	  
	    }
	  //printf("%s %d %d\n", broker_address, *port_in, *port_out);
	  break;
	}           
      cur = cur->xmlChildrenNode;
    }

  if(broker_address == NULL) {
    return -1;
  }

  return 1;
}

xmlDocPtr decode_response(char *pdu)
{
  xmlDocPtr doc = NULL;
  packedobjectsContext *pc;

  pc = init_packedobjects("../schema/packedobjectsd.xsd");
  
  doc = packedobjects_decode(pc, pdu);
  if (pc->decode_error) {
    alert("Failed to decode with error %d.", pc->decode_error);
    return NULL;
  }

  return doc;
}
