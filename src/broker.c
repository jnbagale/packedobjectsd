
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

#include <stdio.h>
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */
#include <inttypes.h> /* for uint64_t */

#include "broker.h"
#include "address.h"

char *get_broker_detail(int node_type, char *address, int port, char *hash_schema)
{
  int rc;
  int size;
  int buffer_size; 
  char *node;
  char *buffer;
  char *endpoint;
  char *broker_address = NULL;
  void *context;
  void *requester;
  Address *addr;
  
  size = strlen(address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  endpoint = malloc(size + 1);
  sprintf(endpoint, "tcp://%s:%d", address, port);

  node = malloc (sizeof(int) + 1);
  sprintf(node, "%d", node_type);
  size = strlen(node);
 
  /* Initialise the zeromq context and socket address */ 
  context = zmq_init (1);

  /* Create socket to connect to look up server*/
  requester = zmq_socket (context, ZMQ_REQ);
  if (requester == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
    return NULL;
  }

  printf("%s: Connecting to the server...\n \n",which_node (node_type));
  rc = zmq_connect (requester, endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
    return NULL;
  }

  rc = send_message_more(requester, node, size); 
  if (rc == -1){
    printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    return NULL;
  }

  rc = send_message(requester, hash_schema, strlen(hash_schema)); 
  
  if (rc == -1){
    printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    return NULL;
  }
  
  buffer = malloc(MAX_BUFFER_SIZE); 
  buffer = receive_message(requester);
  if (buffer == NULL) {
    printf("The received message is NULL\n");
    return NULL;
  }

  addr = make_address_object();
  if(addr == NULL) {
    return NULL;
  }

  buffer_size = deserialize_address(buffer, addr);
  if(buffer_size <= 0) {
    printf("The received address structure could not be decoded\n");
    return NULL;
  }
   
  //printf("Address %s Port In %d Port Out %d\n", addr->address, addr->port_in, addr->port_out);
  size = strlen(addr->address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  broker_address = malloc(size + 1);
  if(node_type == 1) {
    sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_in);
  }
  else if(node_type == 0) {
    sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_out);
  }
    
  /* Freeing up context, socket and pointers */
  free_address_object(addr);
  zmq_close(requester);
  zmq_term(context);
  free(endpoint);
  free(buffer);
  free(node);
  
  return broker_address;
}

/* End of broker.c */
