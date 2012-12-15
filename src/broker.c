
#include <stdio.h>
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */
#include <zmq.h>      /* for ZeroMQ functions */

#include "config.h"
#include "broker.h"
#include "request.h"
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

char *get_broker_detail(char node_type, char *address, int port, char *schema_hash)
{
  int rc;
  int portin;
  int portout;
  int processid;
  int address_size;
  int request_size; 
  int response_size;
  void *context;
  void *requester;
  char *endpoint;
  char *request_pdu;
  char *response_pdu;
  char *broker_address;
  char broker_hostname[MAX_ADDRESS_SIZE];
  xmlDocPtr response_doc = NULL;
 
  address_size = strlen(address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  endpoint = malloc(address_size + 1);
  sprintf(endpoint, "tcp://%s:%d", address, port);
  dbg("Connecting %s to the server at %s",which_node (node_type), endpoint); 

  /* Initialise the zeromq context and socket address */ 
  if((context = zmq_init (1)) == NULL) {
    alert("Failed to initialize zeromq context");
  }

  /* Create socket to connect to look up server*/
  if((requester = zmq_socket (context, ZMQ_REQ)) == NULL){
    alert("Failed to create zeromq socket: %s", zmq_strerror (errno));
    return NULL;
  }
  
  if((rc = zmq_connect (requester, endpoint)) == -1){
    alert("Failed to create zeromq connection: %s", zmq_strerror (errno));
    return NULL;
  }

  if((request_pdu = encode_request(schema_hash, schema_hash, node_type, &request_size)) == NULL) {
    return NULL;
  }

  if((rc = send_message(requester, request_pdu, request_size)) == -1) {
    alert("Failed to send request structure to server: %s", zmq_strerror (errno));
    return NULL;
  }

  if((response_pdu = receive_message(requester, &response_size)) == NULL){
    alert("The received message is NULL\n");
    return NULL;
  }

  if((response_doc = decode_response(response_pdu)) == NULL) {
    return NULL;
  }
  
  if((process_response(response_doc, broker_hostname, &portin, &portout, &processid)) == -1) {
    return NULL;
  }
 
  if((broker_address = malloc(MAX_PDU_SIZE)) == NULL){
    alert("Failed to allocate broker address");
  }

  if(node_type == 'P') {
    sprintf(broker_address, "tcp://%s:%d",broker_hostname, portin);
    dbg("broker endpoint for publisher %s", broker_address);
  }

  if(node_type == 'S') {
    sprintf(broker_address, "tcp://%s:%d",broker_hostname, portout);
    dbg("broker endpoint for subscriber %s", broker_address);
  }

  /* Freeing up zeromq context, socket and pointers */
  free(endpoint);
  free(response_pdu);
  zmq_close(requester);
  zmq_term(context);

  return broker_address;
}

/* End of broker.c */
