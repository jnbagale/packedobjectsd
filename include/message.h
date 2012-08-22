
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

#include <stdio.h>
#include <string.h>    /* for  memcpy() & strlen() */
#include <inttypes.h> /* for int64_t data type  */
#include <zmq.h>     /* for ZeroMQ functions */

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
  addr->address = malloc(strlen(address) + 1);
  strncpy(addr->address, address, strlen(address));

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

#endif
/* End of message.h */
