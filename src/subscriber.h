
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

typedef struct {
  void *context;
  void *subscriber;
  int out_port;
  char *address;
  char *sub_endpoint;
  char *schema_hash;
} subObject;

subObject *make_sub_object()
{
  subObject *sub_obj;

  if ((sub_obj = (subObject *)g_malloc(sizeof(subObject))) == NULL) {
    //g_printerr("failed to malloc subObject!");
    exit(EXIT_FAILURE);
  }

  return sub_obj;
}

void get_broker_sub_address(subObject *sub_obj)
{
  /* Send schema hash to look up server and get broker sub address and port */
  /* Temporary hack to supply port and address of broker */
  sub_obj->out_port = DEFAULT_OUT_PORT;
  sub_obj->address = g_strdup_printf("%s",DEFAULT_ADDRESS);
}

void *subscribe_to_broker(subObject *sub_obj)
{
  int rc;
  uint64_t hwm = 100;
  sub_obj->sub_endpoint =  g_strdup_printf("tcp://%s:%d",sub_obj->address, sub_obj->out_port);

  /* Prepare the context and subscriber socket */
  sub_obj->context = zmq_init (1);
  sub_obj->subscriber = zmq_socket (sub_obj->context, ZMQ_SUB);

  /* Set high water mark to control number of messages buffered */
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_HWM, &hwm, sizeof (hwm));
  assert(rc == 0);

  rc = zmq_connect (sub_obj->subscriber, sub_obj->sub_endpoint);
  assert(rc == 0);
  g_print("Sub_Obj->Subscriber: Successfully connected to SUB socket\n");

  /* Subscribe to group by filtering the received data*/
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_SUBSCRIBE, "", 0);
  assert(rc == 0);
  g_print("Sub_Obj->Subscriber: Ready to receive data from broker %s\n",sub_obj->sub_endpoint);

  g_free(sub_obj->sub_endpoint);
  return sub_obj;
}

char *receive_data(subObject *sub_obj)
{
  zmq_msg_t message;
  zmq_msg_init (&message);
  if (zmq_recv (sub_obj->subscriber, &message, 0))
    return (NULL);
  int size = zmq_msg_size (&message);
  char *data = malloc (size + 1);
  memcpy (data, zmq_msg_data (&message), size);
  zmq_msg_close (&message);
  data [size] = 0;

  return data;
}

void unsubscribe_to_broker(subObject *sub_obj)
{
  zmq_setsockopt (sub_obj->subscriber, ZMQ_UNSUBSCRIBE, "", 0);
  zmq_close (sub_obj->subscriber);
  zmq_term (sub_obj->context);
}

void free_sub_object(subObject *sub_obj)
{
  free(sub_obj->address);
  free(sub_obj->sub_endpoint);
  free(sub_obj);
}
