
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A ZeroMQ broker which receives messages from multiple publishers and forwards subscribers */
/* Binds subscribers to outbound socket */
/* Binds publishers to inbound socket */

#ifndef BROKER_H_
#define BROKER_H_

#include <stdio.h>
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */
#include <inttypes.h> /* for uint64_t */
#include <zmq.h>     /* for ZeroMQ functions */
#include <glib.h>   /* for g_compute_checksum_for_string() */

#include "xmlutils.h"
#include "message.h"

enum ENCODE_TYPE {ENCODED, PLAIN};      /* Supported message encoding types */
enum NODE_TYPE {PUBLISHER, SUBSCRIBER}; /* Supported node types */

static inline char *which_node (int node_type) {
  return ((node_type) ? "SUBSCRIBER" : "PUBLISHER"); 
}

typedef struct {
  void *context;
  void *frontend;
  void *backend;
  int out_port;
  int in_port;
  char *address;
  char *front_endpoint;
  char *back_endpoint;
} brokerObject;

brokerObject *make_broker_object()
{
  brokerObject *broker_obj;

  if ((broker_obj = (brokerObject *)malloc(sizeof(brokerObject))) == NULL) {
    printf("failed to malloc brokerObject!");
    exit(EXIT_FAILURE);
  }

  return broker_obj;
}

brokerObject *init_broker(brokerObject *broker_obj, char *address, int in_port, int out_port)
{
  int size = strlen(address);
  broker_obj->in_port = in_port;
  broker_obj->out_port = out_port;

  broker_obj->address = malloc(size + 1);
  sprintf(broker_obj->address,"%s", address);

  /* To subscribe to all the publishers */
  broker_obj->front_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(broker_obj->front_endpoint, "tcp://%s:%d",broker_obj->address, broker_obj->in_port);

  /* To publish to all the subscribers */
  broker_obj->back_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf( broker_obj->back_endpoint, "tcp://%s:%d",broker_obj->address, broker_obj->out_port);
 
  return broker_obj;
}

void free_broker_object(brokerObject *broker_obj)
{
  if(broker_obj != NULL) {
  zmq_close(broker_obj->frontend);
  zmq_close(broker_obj->backend);
  zmq_term (broker_obj->context);
  free(broker_obj->address);
  free(broker_obj);  
  }
  else {
    printf("The broker_obj struct pointer is NULL\n");
  }
}

void start_broker(brokerObject *broker_obj)
{
  int rc; 
  uint64_t hwm = 100; 

  /* Prepare the context */
  broker_obj->context  = zmq_init (1);
  if (!broker_obj->context){
    printf("Error occurred during zmq_init(): %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Prepare the frontend socket */
  broker_obj->frontend  = zmq_socket (broker_obj->context, ZMQ_SUB);
  if (broker_obj->frontend == NULL){
    printf("Error occurred during zmq_socket() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Prepare the backend socket */
  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_PUB);
  if (broker_obj->backend == NULL){
    printf("Error occurred during zmq_socket() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Bind the frontend socket to subscribe to publishers */
  rc = zmq_bind (broker_obj->frontend,  broker_obj->front_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_bind() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    printf("Broker: Successfully binded to frontend socket at %s\n",  broker_obj->front_endpoint);
    free( broker_obj->front_endpoint);
  }

   /* Subscribe for all the messages from publishers */
  rc = zmq_setsockopt (broker_obj->frontend, ZMQ_SUBSCRIBE, "", 0); 
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Set high water mark to control number of messages buffered for subscribers */
  rc = zmq_setsockopt (broker_obj->backend, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Bind the backend socket to publish to subscribers */
  rc = zmq_bind (broker_obj->backend,  broker_obj->back_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_bind() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    printf("Broker: Successfully binded to backend socket at %s\n", broker_obj->back_endpoint);
    free(broker_obj->back_endpoint);
  }

  /* Start the forwarder device */
  zmq_device (ZMQ_FORWARDER, broker_obj->frontend, broker_obj->backend);

  /* We should never reach here unless something goes wrong! */
}

char *get_broker_detail(int node_type, char *address, int port, char *path_schema)
{
  int rc;
  int size;
  int xml_size;
  int buffer_size; 
  char *node;
  char *buffer;
  char *endpoint;
  char *char_schema;
  char *hash_schema;
  char *broker_address = NULL;
  xmlDoc *doc_schema;
  void *context;
  void *requester;
  Address *addr;
 
  /* Creating MD5 hash of the xml schema */
  doc_schema = init_xmlutils(path_schema); /* Add error checking if xml doesn't exist in given path */
  if(doc_schema == NULL) {
    printf("The XML schema: %s doesn't exist\n", path_schema);
    exit(EXIT_FAILURE);
  }
  char_schema = (char *)xmldoc2string(doc_schema, &xml_size);
  hash_schema = g_compute_checksum_for_string(G_CHECKSUM_MD5, char_schema, strlen(char_schema));

  /* Initialise the zeromq context and socket address */ 
  context = zmq_init (1);
  size = strlen(address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  endpoint = malloc(size + 1);
  sprintf(endpoint, "tcp://%s:%d", address, port);
  node = malloc (sizeof(int));
  sprintf(node,"%d", node_type);
  size = strlen(node);

  /* Create socket to connect to look up server*/
  requester = zmq_socket (context, ZMQ_REQ);
  if (requester == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
    return broker_address ;
  }

  printf("%s: Connecting to the server...\n \n",which_node (node_type));
  rc = zmq_connect (requester, endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
    return broker_address;
  }

  rc = send_message_more(requester, node, size); 
  if (rc == -1){
    printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    return broker_address;
  }

  rc = send_message(requester, hash_schema, strlen(hash_schema)); 
  
  if (rc == -1){
    printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    return broker_address;
  }

  addr = make_address_object();
  buffer = malloc(MAX_BUFFER_SIZE); 
  buffer = receive_message(requester, &size);

  if (buffer != NULL) {
    buffer_size = deserialize_address(buffer, addr);
     if(size != buffer_size) {
      printf("The received address structure could not be decoded\n");
    }
    else {
      //printf("Address %s Port In %d Port Out %d\n", addr->address, addr->port_in, addr->port_out);
      if(node_type == PUBLISHER) {
	broker_address = g_strdup_printf("tcp://%s:%d",addr->address, addr->port_in);
      }
      else if(node_type == SUBSCRIBER) {
	broker_address = g_strdup_printf("tcp://%s:%d",addr->address, addr->port_out);
      }
      printf ("%s: Received broker address: %s\n", which_node(node_type), broker_address);
    }
  }

  /* Freeing up context, socket and pointers */
  zmq_close (requester);
  zmq_term (context);
  free(endpoint);

  return broker_address;
}

#endif
/* End of broker.h */
