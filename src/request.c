
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
#include <zmq.h>  /* for ZeroMQ functions */

#include "request.h"
#include "address.h"
#include "config.h"

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

Request *make_request_object() 
{
  Request *req;
  
  if ((req = (Request *) malloc(sizeof(Request))) == NULL) {
    alert("Failed to allocate Request structure.");
    return NULL;
  }
   
  return req;
}

int serialize_request(char *buffer, Request *req) /* Add host to network order code for port numbers */
{
  size_t offset = 0;

  memcpy(buffer, &req->node_type, sizeof(req->node_type));
  offset = sizeof(req->node_type);
  memcpy(buffer + offset, req->schema_hash, strlen(req->schema_hash) + 1);
  offset = offset + strlen(req->schema_hash) + 1;
  dbg("serialize offset:%d",offset);

  return offset;
}

int deserialize_request(char *buffer, Request *req)  /* Add network to host order code for port numbers */
{
  size_t offset = 0;
   
  if ((req->schema_hash = malloc(MAX_HASH_SIZE)) == NULL) {
    printf("Failed to allocate schema_hash!\n");
    return -1;
  }

  memcpy(&req->node_type, buffer, sizeof(req->node_type));
  offset = sizeof(req->node_type);
  memcpy(req->schema_hash, buffer + offset, strlen(buffer + offset) + 1);
  offset = offset + strlen(buffer + offset) + 1;
  dbg("deserialize offset:%d",offset);

  return offset;
}

void free_request_object(Request *req) 
{
  if(req->schema_hash != NULL) {
    free(req->schema_hash);
  }
  free(req);
}

char *get_broker_detail(char node_type, char *address, int port, char *schema_hash)
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
 
  if((req_buffer = malloc(MAX_BUFFER_SIZE)) == NULL) {
    alert("Failed to allocate request buffer");
  }

  if( (req = make_request_object()) == NULL) {
    alert("Failed to create request object");
    return NULL;
  }

  size = strlen(schema_hash);
  dbg("schema hash length:%d",size);

  if((req->schema_hash = malloc(size + 1)) == NULL) {
    alert("Failed to allocate hash schema");
    return NULL;
  }
  sprintf(req->schema_hash, "%s", schema_hash);
  req->node_type = node_type;
 
  if((ret = serialize_request(req_buffer, req)) == 0) {
    alert("Failed to serialize request structure");
    return NULL;
  }

  if((rc = send_message(requester, req_buffer, ret)) == -1) {
    alert("Failed to send request structure to server: %s", zmq_strerror (errno));
    return NULL;
  }
   
  if((buffer = malloc(MAX_BUFFER_SIZE)) == NULL){
    alert("Failed to allocate hash schema");
    return NULL;
  }

  if((buffer = receive_message(requester, &size)) == NULL){
    alert("The received message is NULL\n");
    return NULL;
  }

  if((addr = make_address_object()) == NULL){
    alert("Failed to create address object");
    return NULL;
  }

  if((buffer_size = deserialize_address(buffer, addr)) <= 0) {
    alert("The received address structure could not be decoded.");
    return NULL;
  }
   
  size = strlen(addr->address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
  if((broker_address = malloc(size + 1)) == NULL){
    alert("Failed to allocate broker address");
  }

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
