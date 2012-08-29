
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
#include <zmq.h>     /* for ZeroMQ functions */

#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <unistd.h>
#define _GNU_SOURCE
#include <crypt.h>  /* for crypt() */

#include "broker.h"
#include "address.h"
#include "message.h"
#include "xmlutils.h"
#include "config.h"

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
    return NULL;
  }
  char_schema = (char *)xmldoc2string(doc_schema, &xml_size);
  hash_schema = crypt(char_schema, "$1$"); /* $1$ is MD5 */
  
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
      size = strlen(addr->address) + sizeof (int) + 7;  /* 7 bytes for 'tcp://' and ':' */
      broker_address = malloc(size + 1);
      if(node_type == PUBLISHER) {
	sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_in);
      }
      else if(node_type == SUBSCRIBER) {
	sprintf(broker_address, "tcp://%s:%d", addr->address, addr->port_out);
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

/* End of broker.c */
