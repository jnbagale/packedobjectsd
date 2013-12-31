
#include <stdio.h>
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */
#include <zmq.h>      /* for ZeroMQ functions */
#include <arpa/inet.h> /* for ntohl() */

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

int getBrokerInfo(packedobjectsdObject *pod_obj)
{
  int rc;
  int64_t more;
  size_t more_size = sizeof(more);
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
  char *nodetype;
  char broker_hostname[MAX_ADDRESS_SIZE];
  xmlDocPtr response_doc = NULL;
 
  address_size = strlen(pod_obj->server_address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  endpoint = malloc(address_size + 1);
  sprintf(endpoint, "tcp://%s:%d", pod_obj->server_address, pod_obj->server_port);

  nodetype =  which_node(pod_obj->node_type);
  dbg("Connecting %s to the server at %s", nodetype, endpoint); 

  /* Initialise the zeromq context and socket address */ 
  if((context = zmq_init (1)) == NULL) {
    alert("Failed to initialize zeromq context");
  }

  /* Create socket to connect to look up server*/
  if((requester = zmq_socket (context, ZMQ_REQ)) == NULL){
    alert("Failed to create zeromq socket: %s", zmq_strerror (errno));
    return -1;
  }
  
  if((rc = zmq_connect (requester, endpoint)) == -1){
    alert("Failed to create zeromq connection: %s", zmq_strerror (errno));
    return -1;
  }
  
  if((request_pdu = encodeRequestDoc(pod_obj->uid_str, pod_obj->schema_hash, nodetype, &request_size)) == NULL) {
    return -1;
  }

  if((rc = sendMessagePDU(requester, request_pdu, request_size, 0)) == -1) {
    alert("Failed to send request structure to server: %s", zmq_strerror (errno));
    return -1;
  }

  // receive node id
  if((response_pdu = receiveMessagePDU(requester, &response_size)) == NULL){
    alert("The received message is NULL\n");
    return -1;
  }
  
  // convert received node id to host order bytes
  pod_obj->unique_id = ntohl(*(unsigned long *)response_pdu);
  dbg("The node_id assigned is %lu", pod_obj->unique_id);

  if((rc = zmq_getsockopt(requester, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  // receive broker details
  if(more) {
  
    if((response_pdu = receiveMessagePDU(requester, &response_size)) == NULL){
      alert("The received message is NULL\n");
      return -1;
    }

    if((response_doc = decodeResponseDoc(response_pdu)) == NULL) {
      return -1;
    }
  
    if((processResponseDoc(response_doc, broker_hostname, &portin, &portout, &processid)) == -1) {
      return -1;
    }
 
    if (pod_obj->node_type == PUBLISHER || pod_obj->node_type == PUBSUB) {
      if((pod_obj->publisher_endpoint = malloc(MAX_PDU_SIZE)) == NULL){
	alert("Failed to allocate memory for publisher endpoint");
	return -1;
      }
      sprintf(pod_obj->publisher_endpoint, "tcp://%s:%d", broker_hostname, portin);
      dbg("broker endpoint for publisher %s", pod_obj->publisher_endpoint);
    }

    if (pod_obj->node_type == SUBSCRIBER || pod_obj->node_type == PUBSUB) {
      if((pod_obj->subscriber_endpoint = malloc(MAX_PDU_SIZE)) == NULL){
	alert("Failed to allocate memory for subscriber endpoint");
	return -1;
      }
      sprintf(pod_obj->subscriber_endpoint, "tcp://%s:%d", broker_hostname, portout);
      dbg("broker endpoint for subscriber %s", pod_obj->subscriber_endpoint);
    }

    if (pod_obj->node_type == SEARCHER || pod_obj->node_type == RESPONDER || pod_obj->node_type == SEARES) {
      if((pod_obj->publisher_endpoint = malloc(MAX_PDU_SIZE)) == NULL){
	alert("Failed to allocate memory for publisher endpoint");
	return -1;
      }
      sprintf(pod_obj->publisher_endpoint, "tcp://%s:%d", broker_hostname, portin);
      dbg("broker endpoint for publisher %s", pod_obj->publisher_endpoint);

      if((pod_obj->subscriber_endpoint = malloc(MAX_PDU_SIZE)) == NULL){
	alert("Failed to allocate memory for subscriber endpoint");
	return -1;
      }
      sprintf(pod_obj->subscriber_endpoint, "tcp://%s:%d", broker_hostname, portout);
      dbg("broker endpoint for subscriber %s", pod_obj->subscriber_endpoint);
    }

  }

  /* Freeing up zeromq context, socket and pointers */
  free(endpoint);
  free(response_pdu);
  zmq_close(requester);
  zmq_term(context);

  return 0;
}

/* End of broker.c */
