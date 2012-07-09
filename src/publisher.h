
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
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for uint64_t */

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
    //g_printerr("failed to malloc pubObject!");
    exit(EXIT_FAILURE);
  }

  return pub_obj;
}

pubObject *publish_to_broker(pubObject *pub_obj)
{
  int rc; 
  uint64_t hwm = 100;
  pub_obj->pub_endpoint =  g_strdup_printf("tcp://%s:%d",pub_obj->address, pub_obj->in_port);

  /* Prepare the context and publisher socket */
  pub_obj->context = zmq_init (1);
  pub_obj->publisher = zmq_socket (pub_obj->context, ZMQ_PUB); 
  rc = zmq_setsockopt (pub_obj->publisher, ZMQ_HWM, &hwm, sizeof (hwm));
  assert(rc == 0);
  rc = zmq_connect (pub_obj->publisher, pub_obj->pub_endpoint);
  assert(rc == 0);
  g_print("Publisher: Successfully connected to PUB socket\n");
  g_print("Publisher: Ready to send data to broker at %s\n",pub_obj->pub_endpoint);

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
