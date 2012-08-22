
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <glib.h>       /* for g_compute_checksum_for_string() */
#include <stdio.h>
#include <string.h>    /* for  memcpy() & strlen() */
#include <inttypes.h> /* for int64_t data type  */
#include <zmq.h>     /* for ZeroMQ functions */

#include "xmlutils.h"

enum ENCODE_TYPE {ENCODED, PLAIN};      /* Supported message encoding types */
enum NODE_TYPE {PUBLISHER, SUBSCRIBER}; /* Supported node types */

static inline char *which_node (int node_type) {
  return ((node_type) ? "SUBSCRIBER" : "PUBLISHER"); 
}

typedef struct {
  long pid;
  unsigned int port_in;
  unsigned int port_out;
  char *address;  

} Address; 


Address *make_address_object(void) 
{

  Address *addr;

  if ((addr = (Address *) malloc(sizeof(Address))) == NULL) {
    printf("Failed to allocate Address structure!\n");
    exit(EXIT_FAILURE);
  }

  return addr;
}

Address *create_address(Address *addr, char *address, int port_in, int port_out, long pid ) 
{
  addr->pid = pid;
  addr->port_in = port_in;
  addr->port_out = port_out;
  addr->address = g_strdup(address);

  return addr;
}

void free_address_object(Address *addr) 
{
  free(addr->address);
  free(addr);
}

int serialize_address(char *buffer, Address *addr) /* Add host to network order code for port numbers */
{
  size_t offset = 0;

  memcpy(buffer, &addr->pid, sizeof(addr->pid));
  offset = sizeof(addr->pid);
  memcpy(buffer + offset, &addr->port_in, sizeof(addr->port_in));
  offset = offset + sizeof(addr->port_in);
  memcpy(buffer + offset, &addr->port_out, sizeof(addr->port_out));
  offset = offset + sizeof(addr->port_out);
  memcpy(buffer + offset, addr->address, strlen(addr->address) + 1);
  offset = offset + strlen(addr->address) + 1;

  return offset;
}

int deserialize_address(char *buffer, Address *addr)  /* Add network to host order code for port numbers */
{
  size_t offset = 0;
   
  if ((addr->address = malloc(MAX_ADDRESS_SIZE)) == NULL) {
    printf("Failed to allocate address!\n");
  }

  memcpy(&addr->pid, buffer, sizeof(addr->pid));
  offset = sizeof(addr->pid);
  memcpy(&addr->port_in, buffer + offset, sizeof(addr->port_in));
  offset = offset + sizeof(addr->port_in);
  memcpy(&addr->port_out, buffer + offset, sizeof(addr->port_out));
  offset = offset + sizeof(addr->port_out);
  memcpy(addr->address, buffer + offset, strlen(buffer + offset) + 1);
  offset = offset + strlen(buffer + offset) + 1;

  return offset;
}

int send_message(void *socket, char *message, int message_length) 
{
  int rc;
  zmq_msg_t z_message;

  rc = zmq_msg_init_size (&z_message, message_length);
  if (rc == -1){
    printf("Error occurred during zmq_msg_init_size(): %s\n", zmq_strerror (errno));
    return rc;
  }

  memcpy (zmq_msg_data (&z_message), message, message_length);
  rc = zmq_send (socket, &z_message, 0); 
  zmq_msg_close (&z_message);

  return rc;
}

int send_message_more(void *socket, char *message, int message_length) 
{
  int rc;
  zmq_msg_t z_message;

  rc = zmq_msg_init_size (&z_message, message_length);
  if (rc == -1){
    printf("Error occurred during zmq_msg_init_size(): %s\n", zmq_strerror (errno));
    return rc;
  }

  memcpy (zmq_msg_data (&z_message), message, message_length);
  rc = zmq_send (socket, &z_message, ZMQ_SNDMORE);   /* Send the message as part */
  zmq_msg_close (&z_message);

  return rc;
}

char *receive_message(void *socket, int *size) 
{
  int rc;
  char *message = NULL;
  zmq_msg_t z_message;

  rc = zmq_msg_init (&z_message);
  if (rc == -1){
    printf("Error occurred during zmq_msg_init_size(): %s\n", zmq_strerror (errno));
    return NULL;
  }

  rc = zmq_recv (socket, &z_message, 0);
  if(rc == -1) {
    printf("Error occurred during zmq_recv(): %s\n", zmq_strerror (errno));
    return NULL;
  }
  else {
    *size = zmq_msg_size (&z_message);
    if(*size > 0) {
      message = malloc(*size + 1);
      memcpy (message, zmq_msg_data (&z_message), *size);
      zmq_msg_close (&z_message);
      message [*size] = 0;
     }
    return message;
  }
}

char *receive_message_more(void *socket, int *size)
{
  int rc;
  int64_t more;
  size_t more_size;
  char *message = NULL;
  more_size = sizeof (more);

  rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);
  if (rc == -1){
    printf("Error occurred during zmq_getsockopt(): %s\n", zmq_strerror (errno));
    return NULL;
  }

  if (more) {
    message =  receive_message(socket, size);
  }

 return message;  
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
/* End of message.h */
