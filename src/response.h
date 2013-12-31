
#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "xmlutils.h"

#define MAX_PDU_SIZE 1000    /* the maximum size for request and response pdu */
#define MAX_ADDRESS_SIZE 500 /* the maximum size for broker address */

xmlDocPtr createResponseDoc(char *broker_address, char *port_in, char *port_out, char *process_id);
char *encodeResponseDoc(char *broker_address, int port_in, int port_out, int process_id, int *response_size);
int processResponseDoc(xmlDocPtr response_doc, char *broker_address, int *port_in, int *port_out, int *process_id);
xmlDocPtr decodeResponseDoc(char *pdu);

#endif
/* End of response.h */
