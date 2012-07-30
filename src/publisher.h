
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <zmq.h>
#include <stdio.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */

#include "xmlutils.h"
#include "packedobjectsd.h"

typedef struct {
  void *context;
  void *publisher;
  int in_port;
  char *address;
  char *pub_endpoint;
 } pubObject;

pubObject *make_pub_object()
{
  pubObject *pub_obj;

  if ((pub_obj = (pubObject *) malloc(sizeof(pubObject))) == NULL) {
    printf("failed to malloc pubObject!\n");
    exit(EXIT_FAILURE);
  }

  return pub_obj;
}

pubObject *publish_to_broker(pubObject *pub_obj, char *path_schema)
{
  /* Retrieve broker's address details from lookup server using the schema */
  get_broker_detail(PUBLISHER, pub_obj->address, path_schema);

  /* Establish Publish connection to the broker using the schema */
  int rc; 
  uint64_t hwm = 100;
  int size = strlen(pub_obj->address);

  pub_obj->pub_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(pub_obj->pub_endpoint, "tcp://%s:%d", pub_obj->address, pub_obj->in_port);
 
  /* Prepare the context and publisher socket */
  pub_obj->context = zmq_init (1);
  pub_obj->publisher = zmq_socket (pub_obj->context, ZMQ_PUB); 
  if (pub_obj->publisher == NULL){
    printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
  }

  rc = zmq_setsockopt (pub_obj->publisher, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
      printf("Error occurred during zmq_setsockopt(): %s\n", zmq_strerror (errno));
    }
  
  rc = zmq_connect (pub_obj->publisher, pub_obj->pub_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_connect(): %s\n", zmq_strerror (errno));
  }

  printf("PUBLISHER: Successfully connected to PUB socket\n");
  printf("PUBLISHER: Ready to send data to broker at %s\n\n",pub_obj->pub_endpoint);
 
  return pub_obj;
}

int send_data(pubObject *pub_obj, char *message, int message_length, int encode_type)
{
  int rc;
  int size;
  char encode[3];
  
  sprintf(encode,"%d", encode_type);
  size = strlen(encode);

  /* Send ENCODE_TYPE as first part of the message */
  rc = send_message_more (pub_obj->publisher, encode, size); 
  if (rc == -1){
    printf("Error occurred while sending encode type: %s\n", zmq_strerror (errno));
    return rc;
  }

  /* Send actual data as second part of the message */
  rc = send_message(pub_obj->publisher, message, message_length);
  if (rc == -1){
    printf("Error occurred while sending the message(): %s\n", zmq_strerror (errno));
  }
 
  return rc;
}

void unpublish_to_broker(pubObject *pub_obj)
{
  zmq_close (pub_obj->publisher);
  zmq_term (pub_obj->context);
}


void free_pub_object(pubObject *pub_obj)
{
  if(pub_obj != NULL) {
    free(pub_obj->address);
    free(pub_obj->pub_endpoint);
    free(pub_obj);
  }
  else {
    printf("The pub_obj struct pointer is NULL\n");
  }
}

#endif
/* End of publisher.h */
