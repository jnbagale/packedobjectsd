
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <zmq.h>
#include <stdio.h>
#include <glib.h>

#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for uint64_t */

#include "xmlutils.h"

typedef struct {
  void *context;
  void *publisher;
  int in_port;
  char *address;
  char *pub_endpoint;
  char *schema_hash;
} pubObject;

pubObject *make_pub_object()
{
  pubObject *pub_obj;

  if ((pub_obj = (pubObject *)g_malloc(sizeof(pubObject))) == NULL) {
    //printf("failed to malloc pubObject!\n");
    exit(EXIT_FAILURE);
  }

  return pub_obj;
}

pubObject *lookup_broker_pub(pubObject *pub_obj, char *path_schema)
{
  int size;
  char *data, *req_endpoint;
  char *char_schema, *hash_schema;
  xmlDoc *doc_schema = NULL;

  /* Creating MD5 hash of the xml schema*/
  doc_schema = init_xmlutils(path_schema);
  char_schema = (char *)xmldoc2string(doc_schema, &size);
  hash_schema = g_compute_checksum_for_string(G_CHECKSUM_MD5, char_schema, strlen(char_schema));

  printf("Connecting to the server....\n \n");
  void *context = zmq_init (1);

  req_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(req_endpoint, "tcp://%s:%d", pub_obj->address, 5555);

  // Socket to talk to server
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, req_endpoint);

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
  free(req_endpoint);

  return pub_obj;
}

pubObject *publish_to_broker(pubObject *pub_obj, char *path_schema)
{
  /* Retrieve broker's address details from lookup server using the schema */
  lookup_broker_pub(pub_obj, path_schema);

  /* Establish Publish connection to the broker using the schema */
  int rc; 
  uint64_t hwm = 100;
  int size = strlen(pub_obj->address);

  pub_obj->pub_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(pub_obj->pub_endpoint, "tcp://%s:%d", pub_obj->address, pub_obj->in_port);
 
  /* Prepare the context and publisher socket */
  pub_obj->context = zmq_init (1);
  pub_obj->publisher = zmq_socket (pub_obj->context, ZMQ_PUB); 
  rc = zmq_setsockopt (pub_obj->publisher, ZMQ_HWM, &hwm, sizeof (hwm));
  assert(rc == 0);
  rc = zmq_connect (pub_obj->publisher, pub_obj->pub_endpoint);
  assert(rc == 0);
  printf("Publisher: Successfully connected to PUB socket\n");
  printf("Publisher: Ready to send data to broker at %s\n",pub_obj->pub_endpoint);

  return pub_obj;
}

int send_data(pubObject *pub_obj, char *message, int msglen, char *encode)
{
  int rc;

  /* Prepare first part of the message */
  zmq_msg_t z_encode;
  rc = zmq_msg_init_size (&z_encode, 1);
  assert(rc ==0);
  memcpy (zmq_msg_data (&z_encode), encode, 1);

  /* Send first part of the message */
  rc = zmq_send (pub_obj->publisher, &z_encode, ZMQ_SNDMORE);
  assert(rc ==0);
  zmq_msg_close (&z_encode);
 
  /* Prepare second part of the message */
  zmq_msg_t z_msg;
  rc = zmq_msg_init_size (&z_msg, msglen);
  assert(rc ==0);
  memcpy (zmq_msg_data (&z_msg), message, msglen);

  /* Send second part of the message */
  rc = zmq_send (pub_obj->publisher, &z_msg, 0);
  assert(rc == 0);
  zmq_msg_close (&z_msg);

  return rc;
}

void unpublish_to_broker(pubObject *pub_obj)
{
  zmq_close (pub_obj->publisher);
  zmq_term (pub_obj->context);
}


void free_pub_object(pubObject *pub_obj)
{
  free(pub_obj->address);
  free(pub_obj->pub_endpoint);
  free(pub_obj);
}
