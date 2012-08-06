
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include <zmq.h>
#include <stdio.h>
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for int64_t */

#include "message.h"

typedef struct {
  void *context;
  void *subscriber;
  int port;
  int encode_type;
  char *message;
  char *address;
  char *sub_endpoint;
} subObject;

subObject *make_sub_object()
{
  subObject *sub_obj;

  if ((sub_obj = (subObject *) malloc(sizeof(subObject))) == NULL) {
    printf("failed to malloc subObject!\n");
    exit(EXIT_FAILURE);
  }

  return sub_obj;
}

void *subscribe_to_broker(subObject *sub_obj, char *path_schema)
{
  int rc;
  uint64_t hwm = 100;

  /* Retrieve broker's address details from lookup server using the schema */
  sub_obj->sub_endpoint = get_broker_detail(SUBSCRIBER, sub_obj->address, sub_obj->port, path_schema);

  if(sub_obj->sub_endpoint == NULL) {
    printf("Broker address received is NULL\n");
    exit(EXIT_FAILURE);
  }
 
  /* Prepare the context and subscriber socket */
  sub_obj->context = zmq_init (1);
  sub_obj->subscriber = zmq_socket (sub_obj->context, ZMQ_SUB);
  if (sub_obj->subscriber == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
  }

  /* Set high water mark to control number of messages buffered */
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
  }

  rc = zmq_connect (sub_obj->subscriber, sub_obj->sub_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
  }
  else {
    printf("SUBSCRIBER: Successfully connected to SUB socket\n");
  }

  /* Subscribe to group by filtering the received data*/
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_SUBSCRIBE, "", 0);
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
  }
  else {
    printf("SUBSCRIBER: Ready to receive data from broker %s\n\n",sub_obj->sub_endpoint);
  }
  /* Free up subscriber address pointer */
  free(sub_obj->sub_endpoint);
  return sub_obj;
}

subObject *receive_data(subObject *sub_obj)
{
   /* Reading first part of the message */
  int size;
  char *encode = NULL;
  encode = receive_message(sub_obj->subscriber, &size);
  if(encode != NULL) {
    sscanf(encode, "%d", &sub_obj->encode_type);
  }

  /* Reading second part of the message if any */
   sub_obj->message = receive_message_more(sub_obj->subscriber, &size);
  
  return sub_obj;
}

void unsubscribe_to_broker(subObject *sub_obj)
{
  zmq_setsockopt (sub_obj->subscriber, ZMQ_UNSUBSCRIBE, "", 0);
  zmq_close (sub_obj->subscriber);
  zmq_term (sub_obj->context);
}

void free_sub_object(subObject *sub_obj)
{
  if (sub_obj != NULL) {
    free(sub_obj->address);
    free(sub_obj);
  }
  else {
    printf("The sub_obj struct pointer is NULL\n");
  }
}

#endif
/* End of subscriber.h */
