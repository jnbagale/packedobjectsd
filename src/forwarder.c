// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

/* A ZeroMQ broker which receives messages from multiple publishers and forwards subscribers */
/* Binds subscribers to outbound socket */
/* Binds publishers to inbound socket */

#include <zmq.h>
#include <glib.h>
#include <stdio.h>
#include <assert.h> /* for assert() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for uint64_t */

#include "config.h"
#include "forwarder.h"

brokerObject *make_broker_object(gchar *address, gint in_port, gint out_port)
{
  brokerObject *broker_obj;

  if ((broker_obj = (brokerObject *)g_malloc(sizeof(brokerObject))) == NULL) {
    //g_printerr("failed to malloc brokerObject!");
    exit(EXIT_FAILURE);
  }

  broker_obj->in_port = in_port;
  broker_obj->out_port = out_port;
  broker_obj->address =  g_strdup_printf("%s",address);
  
  /* To subscribe to all the publishers */
  broker_obj->front_address = g_strdup_printf("tcp://%s:%d",broker_obj->address, broker_obj->in_port);
  
  /* To publish to all the subscribers */
  broker_obj->back_address =  g_strdup_printf("tcp://%s:%d",broker_obj->address, broker_obj->out_port);
 
  return broker_obj;
}

void connect_to_server(gchar *address, gint in_port, gint out_port, gchar *hash_schema)
{
 
  printf("Connecting to the server....\n \n");
  /* networking code to connect to server and */
  /* send hash of schema and network address will come here */
  printf("Sent broker details to the server\n");
}

void start_broker(brokerObject *broker_obj)
{
  gint rc; 
  uint64_t hwm = 100; 

  /* Prepare context and sockets */
  broker_obj->context  = zmq_init (1);
  if (!broker_obj->context){
    g_print("Error occurred during zmq_init(): %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  broker_obj->frontend  = zmq_socket (broker_obj->context, ZMQ_SUB);
  if (broker_obj->frontend == NULL){
    g_print("Error occurred during zmq_socket() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_PUB);
  if (broker_obj->backend == NULL){
    g_print("Error occurred during zmq_socket() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  rc = zmq_bind (broker_obj->frontend,  broker_obj->front_address);
  if (rc == -1){
    g_print("Error occurred during zmq_bind() frontend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    g_print("Broker: Successfully binded to frontend socket at %s\n",  broker_obj->front_address);
    g_free( broker_obj->front_address);
  }

  rc = zmq_setsockopt (broker_obj->backend, ZMQ_HWM, &hwm, sizeof (hwm));
  if (rc == -1){
    g_print("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }

  rc = zmq_bind (broker_obj->backend,  broker_obj->back_address);
  if (rc == -1){
    g_print("Error occurred during zmq_bind() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
  }
  else{
    g_print("Broker: Successfully binded to backend socket at %s\n", broker_obj->back_address);
    g_free(broker_obj->back_address);
  }

  //  Subscribe for everything
  rc = zmq_setsockopt (broker_obj->frontend, ZMQ_SUBSCRIBE, "", 0); 
  if (rc == -1){
    g_print("Error occurred during zmq_setsockopt() backend: %s\n", zmq_strerror (errno));
    free_broker_object(broker_obj);
    exit(EXIT_FAILURE);
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
  g_free(broker_obj->address);
  g_free(broker_obj);  
}
/* End of forwarder.c */
