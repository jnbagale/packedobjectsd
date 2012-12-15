
#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "xmlutils.h"

#define MAX_PDU_SIZE 1000    /* the maximum size for request and response pdu */
#define MAX_ADDRESS_SIZE 500 /* the maximum size for broker address */

xmlDocPtr create_response(char *broker_address, char *port_in, char *port_out, char *process_id);
char *encode_response(char *broker_address, int port_in, int port_out, int process_id, int *response_size);
int process_response(xmlDocPtr response_doc, char *broker_address, int *port_in, int *port_out, int *process_id);
xmlDocPtr decode_response(char *pdu);

#endif
/* End of response.h */
