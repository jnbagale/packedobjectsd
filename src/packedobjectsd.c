
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
#include <zmq.h>

#include "broker.h"
#include "message.h"
#include "xmlutils.h"
#include "packedobjectsd.h"


static void *packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *path_schema);
static packedobjectsdObject *packedobjectsd_publish(packedobjectsdObject *pod_obj, char *path_schema);

packedobjectsdObject *packedobjectsd_init(int node_type, char *path_schema, char *server_address, int server_port)
{
  packedobjectsdObject *pod_obj;
  if ((pod_obj = (packedobjectsdObject *) malloc(sizeof(packedobjectsdObject))) == NULL) {
    printf("failed to malloc packedobjectsdObject!\n");
    return NULL;
  }

  pod_obj->server_port = server_port;
  int size = strlen(server_address);
  pod_obj->server_address = malloc( size + 1);
  sprintf(pod_obj->server_address, server_address, size);

  pod_obj->pc = NULL;
  pod_obj->pc = init_packedobjects((const char *) path_schema); /* check if pod_obj->pc is NULL */

  switch (node_type) {
  case 0:
    packedobjectsd_subscribe(pod_obj, path_schema);
    break;

  case 1:
    packedobjectsd_publish(pod_obj, path_schema);
    break;
   
  case 2:
    packedobjectsd_publish(pod_obj, path_schema);
    packedobjectsd_subscribe(pod_obj, path_schema);
      
    break;
  default:
    printf("Invalid node type\n"); /* Handle this properly */
   
  }

  return pod_obj;
}

static void *packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *path_schema)
{
  int rc;
  uint64_t hwm = 100;

  /* Retrieve broker's address details from lookup server using the schema */
  pod_obj->subscriber_endpoint = get_broker_detail(SUBSCRIBER, pod_obj->server_address, pod_obj->server_port, path_schema);

  if(pod_obj->subscriber_endpoint == NULL) {
    printf("Broker address received is NULL\n");
    return NULL;
  }
 
  /* Prepare the zeromq subscriber context and socket */
  pod_obj->subscriber_context = zmq_init (1);
  pod_obj->subscriber_socket = zmq_socket (pod_obj->subscriber_context, ZMQ_SUB);
  if (pod_obj->subscriber_socket == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
  }

  /* Set high water mark to control number of messages buffered */
  rc = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
  }

  rc = zmq_connect (pod_obj->subscriber_socket, pod_obj->subscriber_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
  }
  else {
    printf("SUBSCRIBER: Successfully connected to SUB socket\n");
  }

  /* Podscribe to group by filtering the received data*/
  rc = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, "", 0);
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
  }
  else {
    printf("SUBSCRIBER: Ready to receive data from broker %s\n\n",pod_obj->subscriber_endpoint);
  }

  free(pod_obj->subscriber_endpoint); /* Free up subscriber endpoint pointer */
  return pod_obj;
}

static packedobjectsdObject *packedobjectsd_publish(packedobjectsdObject *pod_obj, char *path_schema)
{
  int rc; 
  uint64_t hwm = 100;

  /* Retrieve broker's address details from lookup server using the schema */
  pod_obj->publisher_endpoint = get_broker_detail(PUBLISHER, pod_obj->server_address, pod_obj->server_port, path_schema);

  if(pod_obj->publisher_endpoint == NULL) {
    printf("Broker address received is NULL\n");
    return NULL;
  }

  /* Prepare the context and podlisher socket */
  pod_obj->publisher_context = zmq_init (1);
  pod_obj->publisher_socket = zmq_socket (pod_obj->publisher_context, ZMQ_PUB); 
  if (pod_obj->publisher_socket == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
  }

  rc = zmq_setsockopt (pod_obj->publisher_socket, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
  }
  
  rc = zmq_connect (pod_obj->publisher_socket, pod_obj->publisher_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
  }

  printf("PUBLISHER: Successfully connected to PUB socket\n");
  printf("PUBLISHER: Ready to send data to broker at %s\n\n",pod_obj->publisher_endpoint);

  free(pod_obj->publisher_endpoint); /* Free up publisher endpoint pointer */
  return pod_obj;
}

xmlDocPtr receive_data(packedobjectsdObject *pod_obj)
{
  /* Reading first part of the message */
  int size;
  char *pdu = NULL;
  char *encode = NULL;
  xmlDocPtr doc = NULL;

  encode = receive_message(pod_obj->subscriber_socket, &size);
  if(encode != NULL) {
    sscanf(encode, "%d", &pod_obj->encode_type);
  }

  /* Reading second part of the message if any */
  pdu = receive_message_more(pod_obj->subscriber_socket, &size);

  if(pod_obj->encode_type == ENCODED) {
    doc = packedobjects_decode(pod_obj->pc, pdu);
    size = pod_obj->pc->bytes;
    printf("Encoded pdu %s,bytes:%d\n", pdu, size);
  }
  else {
    size = strlen(pdu);
    doc = (xmlDoc *) xmlstring2doc(pdu, size);
    printf("Plain pdu %s,bytes:%d\n", pdu, size);
  }

  return doc;
}

int send_data(packedobjectsdObject *pod_obj, xmlDocPtr doc, int encode_type)
{
  int rc;
  int size;
  char encode[3];
  char *pdu = NULL;

  sprintf(encode,"%d", encode_type);
  size = strlen(encode);

  /* Send ENCODE_TYPE as first part of the message */
  rc = send_message_more (pod_obj->publisher_socket, encode, size); 
  if (rc == -1){
    printf("Error occurred while sending encode type: %s\n", zmq_strerror (errno));
    return rc;
  }

  /* Send actual data as second part of the message */
  if(encode_type == ENCODED) {
    if (pod_obj->pc) {
      pdu = packedobjects_encode(pod_obj->pc, doc);
      size =  pod_obj->pc->bytes;
      printf("Encoded pdu %s,bytes:%d\n", pdu, size);
    }
  }
  else {
    pdu = (char *)xmldoc2string(doc, &size);
    printf("Plain pdu %s, bytes:%d\n", pdu, size);
  }

  rc = send_message(pod_obj->publisher_socket, pdu, size);
  if (rc == -1){
    printf("Error occurred while sending the message(): %s\n", zmq_strerror (errno));
  }
   
  
  return rc;
}

void packedobjectsd_free(packedobjectsdObject *pod_obj)
{
  if (pod_obj != NULL) {
    if(pod_obj->node_type == 0 || pod_obj->node_type == 2 ) {
      zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_UNSUBSCRIBE, "", 0);
      zmq_close (pod_obj->subscriber_socket);
      zmq_term (pod_obj->subscriber_context);
    }

    if(pod_obj->node_type == 1 || pod_obj->node_type == 2) {
      zmq_close (pod_obj->publisher_socket);
      zmq_term (pod_obj->publisher_context);
    }

    free_packedobjects(pod_obj->pc);
    free(pod_obj->server_address);
    free(pod_obj);
  }
}

/* End of packedobjectsd.c */
