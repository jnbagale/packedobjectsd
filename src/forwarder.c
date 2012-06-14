// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

/* ZeroMQ Forwarder which receives data from publishers and forwards to subscribers */
/* Binds PUB socket to tcp://\*:5556 */
/* Binds SUB socket to given host address */

#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <stdlib.h> /* for exit()   */

#include "config.h"
#include "forwarder.h"

brokerObject *make_broker_object(void)
{
  brokerObject *broker_obj;

  if ((broker_obj = (brokerObject *)g_malloc(sizeof(brokerObject))) == NULL) {
    //g_printerr("failed to malloc brokerObject!");
    exit(EXIT_FAILURE);
  }

  return broker_obj;
}


void start_broker(brokerObject *broker_obj)
{
  gint rc_in; 
  gint rc_out;

  // To subscribe to all the publishers
  gchar *frontend_endpoint = g_strdup_printf("tcp://*:%d",broker_obj->pub_port);

  // To publish to all the potential subscribers
  gchar *backend_endpoint =  g_strdup_printf("tcp://%s:%d",broker_obj->broker, broker_obj->sub_port);

  //  Prepare context and sockets
  broker_obj->context  = zmq_init (1);
  broker_obj->frontend  = zmq_socket (broker_obj->context, ZMQ_SUB);
  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_PUB);
  
  rc_in = zmq_bind (broker_obj->frontend,  frontend_endpoint);
  assert(rc_in == 0);
  g_print("Broker: Successfully binded to inbound socket\n");

  rc_out = zmq_bind (broker_obj->backend, backend_endpoint);
  assert(rc_out == 0);
  g_print("Broker: Successfully binded to outbound socket\n");

  //  Subscribe for everything
  zmq_setsockopt (broker_obj->frontend, ZMQ_SUBSCRIBE, "", 0); 
  g_print("Broker: Receiving messages from publishers at %s\n",frontend_endpoint);
  g_print("Broker: Forwarding messages to subcribers from %s\n",backend_endpoint);
  //  Start the forwarder device
  zmq_device (ZMQ_FORWARDER, broker_obj->frontend, broker_obj->backend);
}

void free_broker_object(brokerObject *broker_obj)
{
  zmq_term (broker_obj->context);
  g_free(broker_obj->broker);
  g_free(broker_obj);  
}
/* End of forwarder.c */
