
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
#include <zmq.h>  /* for ZeroMQ functions */

#include "message.h"
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

int send_message(void *socket, char *message, int message_length) 
{
  int rc;
  zmq_msg_t z_message;

  if((rc = zmq_msg_init_size (&z_message, message_length)) == -1){
    alert("Error occurred during zmq_msg_init_size(): %s", zmq_strerror (errno));
    return rc;
  }

  memcpy (zmq_msg_data (&z_message), message, message_length);
  if((rc = zmq_send (socket, &z_message, 0)) == -1){
    alert("Error occurred during zmq_send(): %s", zmq_strerror (errno));
  }
  zmq_msg_close (&z_message);
  dbg("size of message sent: %d bytes",message_length);
  return rc;
}

char *receive_message(void *socket, int *message_length) 
{
  int rc;
  int size;
  *message_length = -1;
  char *message = NULL;
  zmq_msg_t z_message;

  if((rc = zmq_msg_init (&z_message)) == -1){
    alert("Error occurred during zmq_msg_init_size(): %s", zmq_strerror (errno));
    return NULL;
  }

  if((rc = zmq_recv(socket, &z_message, 0)) == -1){
    alert("Error occurred during zmq_recv(): %s", zmq_strerror (errno));
    return NULL;
  }

  size = zmq_msg_size (&z_message);
  if(size > 0) {
    if((message = malloc(size + 1)) == NULL){
      alert("Failed to allocated message");
      return NULL;
    }
    memcpy (message, zmq_msg_data (&z_message), size);
    zmq_msg_close (&z_message);
    message [size] = '\0';   
    *message_length = size;
  }
  dbg("size of message received: %d bytes",size);
  return message;
}

/* End of message.c */
