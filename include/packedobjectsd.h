
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef PACKEDOBJECTSD_H_
#define PACKEDOBJECTSD_H_

#include <stdio.h>
#include <string.h>     /* for strlen() */
#include <stdlib.h>    /* for exit()   */
#include <inttypes.h> /* for int64_t */
#include <zmq.h>     /* ZeroMQ functions   */

#include "broker.h"

typedef struct {
  void *context;
  void *subscriber;
  int port;
  int encode_type;
  char *message;
  char *address;
  char *sub_endpoint;
} subObject;

typedef struct {
  void *context;
  void *publisher;
  int port;
  char *address;
  char *pub_endpoint;
 } pubObject;

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
    exit(EXIT_FAILURE); /* Handle it gently. Don't exit here. return sthg to user */
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
  int rc; 
  uint64_t hwm = 100;

  /* Retrieve broker's address details from lookup server using the schema */
  pub_obj->pub_endpoint = get_broker_detail(PUBLISHER, pub_obj->address, pub_obj->port, path_schema);

  if(pub_obj->pub_endpoint == NULL) {
    printf("Broker address received is NULL\n");
    exit(EXIT_FAILURE);
  }

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
/* End of packedobjectsd.h */
