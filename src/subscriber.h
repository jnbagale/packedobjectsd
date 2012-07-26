
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
  char *encode;
  char *message;
  char *address;
  char *sub_endpoint;
} subObject;

subObject *make_sub_object()
{
  subObject *sub_obj;

  if ((sub_obj = (subObject *)malloc(sizeof(subObject))) == NULL) {
    //printf("failed to malloc subObject!\n");
    exit(EXIT_FAILURE);
  }

  return sub_obj;
}

subObject *lookup_broker_sub(subObject *sub_obj, char *path_schema)
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
  sprintf(req_endpoint, "tcp://%s:%d", sub_obj->address, 5555);

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

  return sub_obj;
}

void *subscribe_to_broker(subObject *sub_obj, char *path_schema)
{

  /* Retrieve broker's address details from lookup server using the schema */
  lookup_broker_sub(sub_obj, path_schema);

  /* Establish Subscribe connection to the broker using the schema */
  int rc;
  uint64_t hwm = 100;
  int size = strlen(sub_obj->address);

  sub_obj->sub_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(sub_obj->sub_endpoint, "tcp://%s:%d",sub_obj->address, sub_obj->out_port);

  /* Prepare the context and subscriber socket */
  sub_obj->context = zmq_init (1);
  sub_obj->subscriber = zmq_socket (sub_obj->context, ZMQ_SUB);

  /* Set high water mark to control number of messages buffered */
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_HWM, &hwm, sizeof (hwm));
  assert(rc == 0);

  rc = zmq_connect (sub_obj->subscriber, sub_obj->sub_endpoint);
  assert(rc == 0);
  printf("Sub_Obj->Subscriber: Successfully connected to SUB socket\n");

  /* Subscribe to group by filtering the received data*/
  rc = zmq_setsockopt (sub_obj->subscriber, ZMQ_SUBSCRIBE, "", 0);
  assert(rc == 0);
  printf("Sub_Obj->Subscriber: Ready to receive data from broker %s\n",sub_obj->sub_endpoint);

  free(sub_obj->sub_endpoint);
  return sub_obj;
}

subObject *receive_data(subObject *sub_obj)
{
  /* Reading first part of the message */
  zmq_msg_t message;
  zmq_msg_init (&message);
  if (zmq_recv (sub_obj->subscriber, &message, 0))
    return (NULL);
  int size = zmq_msg_size (&message);
  
  sub_obj->encode = malloc(size + 1);
  memcpy ( sub_obj->encode, zmq_msg_data (&message), size);
  zmq_msg_close (&message);
  sub_obj->encode [size] = 0;
 
  /* Reading second part of the message if any */
  int64_t more;
  size_t more_size = sizeof (more);
  zmq_getsockopt (sub_obj->subscriber, ZMQ_RCVMORE, &more, &more_size);
  if (more) {
    zmq_msg_t message;
    zmq_msg_init (&message);
    if (zmq_recv (sub_obj->subscriber, &message, 0))
      return (NULL);
    int size = zmq_msg_size (&message);

    sub_obj->message = malloc (size + 1);
    memcpy (sub_obj->message, zmq_msg_data (&message), size);
    zmq_msg_close (&message);
    sub_obj->message [size] = 0;
  }

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
  free(sub_obj->address);
  free(sub_obj->sub_endpoint);
  free(sub_obj);
}
