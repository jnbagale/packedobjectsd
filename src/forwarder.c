
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

#include <zmq.h>
#include <stdio.h>
#include <glib.h>
#include <string.h> /* for strncat() */
#include <assert.h> /* for assert() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for uint64_t */

#include "config.h"
#include "xmlutils.h"
#include "forwarder.h"

brokerObject *make_broker_object()
{
  brokerObject *broker_obj;

  if ((broker_obj = (brokerObject *)malloc(sizeof(brokerObject))) == NULL) {
    //printferr("failed to malloc brokerObject!");
    exit(EXIT_FAILURE);
  }

  return broker_obj;
}

brokerObject *init_broker(brokerObject *broker_obj, char *address, int in_port, int out_port)
{
  int size = strlen(address);
  broker_obj->in_port = in_port;
  broker_obj->out_port = out_port;

  broker_obj->address = malloc(size + 1);
  memcpy(broker_obj->address, address, size);

  /* To subscribe to all the publishers */
  broker_obj->front_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(broker_obj->front_endpoint, "tcp://%s:%d",broker_obj->address, broker_obj->in_port);

  /* To publish to all the subscribers */
  broker_obj->back_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf( broker_obj->back_endpoint, "tcp://%s:%d",broker_obj->address, broker_obj->out_port);
  return broker_obj;
}

void connect_to_server(brokerObject *broker_obj, char *path_schema)
{
  int size;
  char *data;
  char *char_schema, *hash_schema;
  xmlDoc *doc_schema = NULL;

  /* Creating MD5 hash of the xml schema*/
  doc_schema = init_xmlutils(path_schema);
  char_schema = (char *)xmldoc2string(doc_schema, &size);
  hash_schema = g_compute_checksum_for_string(G_CHECKSUM_MD5, char_schema, strlen(char_schema));

  printf("Connecting to the server....\n \n");
  void *context = zmq_init (1);

  // Socket to talk to server
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, "tcp://127.0.0.1:5555");

  zmq_msg_t request;
  zmq_msg_init_size (&request, strlen(hash_schema));
  memcpy (zmq_msg_data (&request), hash_schema, strlen(hash_schema));
  printf ("Sending schema hash to the server: %s\n",hash_schema);
  zmq_send (requester, &request, 0);
  zmq_msg_close (&request);

  zmq_msg_t reply;
  zmq_msg_init (&reply);
  zmq_recv (requester, &reply, 0);

  size = zmq_msg_size (&reply);
  data = malloc(size + 1);
  memcpy ( data, zmq_msg_data (&reply), size);
  data[size] = 0;

  printf ("Received broker address: %s\n",data);
  zmq_msg_close (&reply);

}

void start_broker(brokerObject *broker_obj)
{
  int rc; 
  uint64_t hwm = 100; 

  /* Prepare the context */
  broker_obj->context  = zmq_init (1);
  if (!broker_obj->context){
    printf("Error occurred during zmq_init(): %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Prepare the frontend socket */
  broker_obj->frontend  = zmq_socket (broker_obj->context, ZMQ_SUB);
  if (broker_obj->frontend == NULL){
    printf("Error occurred during zmq_socket() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Prepare the backend socket */
  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_PUB);
  if (broker_obj->backend == NULL){
    printf("Error occurred during zmq_socket() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Bind the frontend socket to subscribe to publishers */
  rc = zmq_bind (broker_obj->frontend,  broker_obj->front_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_bind() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    printf("Broker: Successfully binded to frontend socket at %s\n",  broker_obj->front_endpoint);
    free( broker_obj->front_endpoint);
  }

   /* Subscribe for all the messages from publishers */
  rc = zmq_setsockopt (broker_obj->frontend, ZMQ_SUBSCRIBE, "", 0); 
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Set high water mark to control number of messages buffered for subscribers */
  rc = zmq_setsockopt (broker_obj->backend, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    printf("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  /* Bind the backend socket to publish to subscribers */
  rc = zmq_bind (broker_obj->backend,  broker_obj->back_endpoint);
  if (rc == -1){
    printf("Error occurred during zmq_bind() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    printf("Broker: Successfully binded to backend socket at %s\n", broker_obj->back_endpoint);
    free(broker_obj->back_endpoint);
  }

  /* Start the forwarder device */
  zmq_device (ZMQ_FORWARDER, broker_obj->frontend, broker_obj->backend);
  /* We should never reach here unless something goes wrong! */
}

void free_broker_object(brokerObject *broker_obj)
{
  zmq_close(broker_obj->frontend);
  zmq_close(broker_obj->backend);
  zmq_term (broker_obj->context);
  free(broker_obj->address);
  free(broker_obj);  
}
/* End of forwarder.c */
