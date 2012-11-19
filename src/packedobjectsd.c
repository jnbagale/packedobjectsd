
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <stdio.h>
#include <string.h>     /* for strlen() */
#include <stdlib.h>    /* for exit()   */
#include <inttypes.h> /* for uint64_t */
#include <zmq.h>  /* for ZeroMQ functions */

#include "request.h"
#include "xmlutils.h"
#include "packedobjectsd.h"

#ifdef DEBUG_MODE

#define dbg(fmtstr, args...)					\
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

static packedobjectsdObject *packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash);
static packedobjectsdObject *packedobjectsd_publish(packedobjectsdObject *pod_obj, char *schema_hash);

packedobjectsdObject *init_packedobjectsd(const char *schema_file)
{
  char *schema_hash;
  packedobjectsdObject *pod_obj;
 
  if ((pod_obj = (packedobjectsdObject *) malloc(sizeof(packedobjectsdObject))) == NULL) {
    alert("failed to malloc packedobjectsdObject.");
    return NULL;
  }

  /* Initialise default values for initialisation */
  pod_obj->error_code = 0;
  pod_obj->node_type = 'B';
  pod_obj->server_port = DEFAULT_SERVER_PORT ; 
  pod_obj->server_address = DEFAULT_SERVER_ADDRESS;
  pod_obj->pc = init_packedobjects(schema_file); 
  if(pod_obj->pc == NULL) {
    // alert("Failed to initialise libpackedobjects.");
    pod_obj->error_code = INIT_PO_FAILED;
    return NULL;
  }

  if((schema_hash = xmlfile2hash(schema_file)) == NULL) { /* Creat MD5 Hash of the XML schmea */
    // alert("Failed to create hash of the schema file.");
    pod_obj->error_code =  INVALID_SCHEMA_FILE;
    return NULL;
  }
  dbg("schema_hash: %s", schema_hash);
 
  switch (pod_obj->node_type) {
  case 'S':
    pod_obj = packedobjectsd_subscribe(pod_obj, schema_hash);
    if(pod_obj == NULL) {
      //alert("Failed to subscribe to packedobjectsd");
      pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }
    break;

  case 'P':
    pod_obj = packedobjectsd_publish(pod_obj, schema_hash);
    if(pod_obj == NULL) {
      //alert("Failed to publish to packedobjectsd");
      pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    break;
   
  case 'B':
    pod_obj = packedobjectsd_publish(pod_obj, schema_hash);
    if(pod_obj == NULL) {
      //alert("Failed to publish to packedobjectsd");
      pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    pod_obj = packedobjectsd_subscribe(pod_obj, schema_hash);
    if(pod_obj == NULL) {
      //alert("Failed to subscribe to packedobjectsd");
      pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    break;
  default:
    //alert("Invalid node type."); /* Handle this properly */
    pod_obj->error_code = INVALID_NODE_TYPE;
    return NULL;
  }
   
  return pod_obj;
}

static packedobjectsdObject *packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash)
{
  int rc;
  uint64_t hwm = 100;
  
  /* Retrieve broker's address details from lookup server using the schema */
  pod_obj->subscriber_endpoint = get_broker_detail('S', pod_obj->server_address, pod_obj->server_port, schema_hash);

  if(pod_obj->subscriber_endpoint == NULL) {
    alert("Broker address received is NULL.");
    return NULL;
  }
  else {
    dbg("Broker address receieved for subscriber: %s", pod_obj->subscriber_endpoint);
  }
 
  /* Prepare the zeromq subscriber context and socket */
  if ((pod_obj->subscriber_context = zmq_init(1)) == NULL){
    alert("Failed to initialise zeromq context for subscriber: %s", zmq_strerror (errno));
    return NULL;
  }

  if((pod_obj->subscriber_socket = zmq_socket (pod_obj->subscriber_context, ZMQ_SUB)) == NULL) {
    alert("Failed to create zeromq socket for subscriber: %s", zmq_strerror (errno));
    return NULL;
  }

  /* Set high water mark to control number of messages buffered by subscriber */
  if((rc = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_HWM, &hwm, sizeof (hwm))) == -1) {
    alert("Failed to set zeromq socket options for subscriber: %s", zmq_strerror (errno));
    return NULL;
  }

  if((rc = zmq_connect (pod_obj->subscriber_socket, pod_obj->subscriber_endpoint)) == -1){
    alert("Failed to create zeromq socket for subscriber: %s", zmq_strerror (errno));
    return NULL;
  }
  
  /* Podscribe to group by filtering the received data*/
  rc = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, "", 0);
  if (rc == -1){
    alert("Failed to set subscribe filter for subscriber: %s", zmq_strerror (errno));
    return NULL;
  }
  else {
    dbg("Subscriber is ready to receive data from broker %s",pod_obj->subscriber_endpoint);
  }

  free(pod_obj->subscriber_endpoint); /* Free up subscriber endpoint pointer */
  return pod_obj;
}

static packedobjectsdObject *packedobjectsd_publish(packedobjectsdObject *pod_obj, char *schema_hash)
{
  int rc; 
  uint64_t hwm = 100;
  
  /* Retrieve broker's address details from lookup server using the schema */
  pod_obj->publisher_endpoint = get_broker_detail('P', pod_obj->server_address, pod_obj->server_port, schema_hash);

  if(pod_obj->publisher_endpoint == NULL) {
    alert("Broker address received is NULL");
    return NULL;
  }
  else {
    dbg("Broker address receieved for publisher: %s", pod_obj->publisher_endpoint);
  }

  /* Prepare the context and podlisher socket */
  if (( pod_obj->publisher_context = zmq_init(1)) == NULL){
    alert("Failed to initialise zeromq context for publisher: %s", zmq_strerror (errno));
    return NULL;
  }

  if((pod_obj->publisher_socket = zmq_socket (pod_obj->publisher_context, ZMQ_PUB)) == NULL){ 
    alert("Failed to create zeromq socket for publisher: %s", zmq_strerror (errno));
    return NULL;
  }

  if((rc = zmq_setsockopt (pod_obj->publisher_socket, ZMQ_HWM, &hwm, sizeof (hwm))) == -1){
    alert("Failed to set zeromq socket options for publisher: %s", zmq_strerror (errno));
    return NULL;
  }

  if((rc = zmq_connect (pod_obj->publisher_socket, pod_obj->publisher_endpoint)) == -1){
    alert("Failed to connect publisher: %s\n", zmq_strerror (errno));
    return NULL;
  }

  dbg("Publisher is ready to send data to broker at %s",pod_obj->publisher_endpoint);

  free(pod_obj->publisher_endpoint); /* Free up publisher endpoint pointer */
  return pod_obj;
}

xmlDocPtr packedobjectsd_receive(packedobjectsdObject *pod_obj)
{
  /* Reading the received message */
  int size;
  char *pdu = NULL;
  xmlDocPtr doc = NULL;

  if((pdu = receive_message(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }

  doc = packedobjects_decode(pod_obj->pc, pdu);
  if (pod_obj->pc->decode_error) {
    // alert("Failed to decode with error %d.", pod_obj->pc->decode_error);
    pod_obj->error_code = DECODE_FAILED;
    return NULL;
  }
  dbg("data received and decoded");
  
  free(pdu);
  return doc;
}

int packedobjectsd_send(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;
  int size;
  char *pdu = NULL;

  pdu = packedobjects_encode(pod_obj->pc, doc);
  size =  pod_obj->pc->bytes;
  if (size == -1) {
    // fprintf(stderr, "Failed to encode with error %d.\n", pod_obj->pc->encode_error);
    pod_obj->error_code = ENCODE_FAILED;
      return size;
    }

  if((rc = send_message(pod_obj->publisher_socket, pdu, size)) == -1) {
    //alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }
 
  dbg("data encoded and sent");
 
  return rc;
}

void free_packedobjectsd(packedobjectsdObject *pod_obj)
{
  if (pod_obj != NULL) {
    if(pod_obj->node_type == 'S' || pod_obj->node_type == 'B' ) {
      zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_UNSUBSCRIBE, "", 0);
      zmq_close (pod_obj->subscriber_socket);
      zmq_term (pod_obj->subscriber_context);
    }

    if(pod_obj->node_type == 'P' || pod_obj->node_type == 'B') {
      zmq_close (pod_obj->publisher_socket);
      zmq_term (pod_obj->publisher_context);
    }

    free_packedobjects(pod_obj->pc);
    free(pod_obj);
    dbg("freeing packedobjectsd is successful");
  }
}

/* End of packedobjectsd.c */
