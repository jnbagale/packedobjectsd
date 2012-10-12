
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
#include <string.h>     /* for strncat() & memcpy() */
#include <stdlib.h>    /* for exit()   */

#include "request.h"
#include "address.h"

Request *make_request_object() 
{
  Request *req;
  
  if ((req = (Request *) malloc(sizeof(Request))) == NULL) {
    printf("Failed to allocate Request structure!\n");
    return NULL;
  }
   
  return req;
}

int serialize_request(char *buffer, Request *req) /* Add host to network order code for port numbers */
{
  size_t offset = 0;

  memcpy(buffer, &req->node_type, sizeof(req->node_type));
  offset = sizeof(req->node_type);
  memcpy(buffer + offset, req->hash_schema, strlen(req->hash_schema) + 1);
  offset = offset + strlen(req->hash_schema) + 1;
 
  return offset;
}

int deserialize_request(char *buffer, Request *req)  /* Add network to host order code for port numbers */
{
  size_t offset = 0;
   
  if ((req->hash_schema = malloc(MAX_HASH_SIZE)) == NULL) {
    printf("Failed to allocate hash_schema!\n");
    return -1;
  }

  memcpy(&req->node_type, buffer, sizeof(req->node_type));
  offset = sizeof(req->node_type);
  memcpy(req->hash_schema, buffer + offset, strlen(buffer + offset) + 1);
  offset = offset + strlen(buffer + offset) + 1;

  return offset;
}

void free_request_object(Request *req) 
{
  if(req->hash_schema != NULL) {
    free(req->hash_schema);
  }
  free(req);
}

char *get_broker_detail(char node_type, char *address, int port, char *hash_schema)
{
  int rc;
  int ret;
  int size;
  int buffer_size; 
  char *endpoint;
  char *buffer = NULL;
  char *req_buffer = NULL;
  char *broker_address = NULL;
  void *context;
  void *requester;
  Address *addr;
  Request *req;

  size = strlen(address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  endpoint = malloc(size + 1);
  sprintf(endpoint, "tcp://%s:%d", address, port);

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
 
  req_buffer = malloc(MAX_BUFFER_SIZE);
  if( (req = make_request_object()) == NULL) {
    return NULL;
  }

  size = strlen(hash_schema);
  if((req->hash_schema = malloc(size + 1)) == NULL) {
    printf("Failed to allocate hash schema!\n");
    return NULL;
  }
  sprintf(req->hash_schema, "%s", hash_schema);
  req->node_type = node_type;
 
  if((ret = serialize_request(req_buffer, req)) == 0) {
    return NULL;
  }

  rc = send_message(requester, req_buffer, ret); 
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

  if((buffer_size = deserialize_address(buffer, addr)) <= 0) {
    printf("The received address structure could not be decoded\n");
    return NULL;
  }
   
  //printf("Address %s Port In %d Port Out %d\n", addr->address, addr->port_in, addr->port_out);
  size = strlen(addr->address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  broker_address = malloc(size + 1);
  if(node_type == 'P') {
    sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_in);
  }
  else if(node_type == 'S') {
    sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_out);
  }
    
  /* Freeing up context, socket and pointers */
  free_address_object(addr);
  zmq_close(requester);
  zmq_term(context);
  free(endpoint);
  free(buffer);
  free(req); 
  free(req_buffer);

  return broker_address;
}

/* End of request.c */
