
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
#include <string.h>    /* for  memcpy() & strlen() */
#include <inttypes.h> /* for int64_t data type  */
#include <zmq.h>     /* for ZeroMQ functions */

#include "message.h"

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

/* End of message.c */
