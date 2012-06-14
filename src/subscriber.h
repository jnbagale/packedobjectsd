// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */

void *subscribe_to_broker(gchar *broker_address, gint broker_sub_port)
{
  gint rc;
  void *subscriber;
  void *context = zmq_init (1);
  gchar *forwarder_address =  g_strdup_printf("tcp://%s:%d",broker_address, broker_sub_port);
  
   /* Socket to subscribe to broker */
  subscriber = zmq_socket (context, ZMQ_SUB);
  rc = zmq_connect (subscriber, forwarder_address);
  assert(rc == 0);
  g_print("Subscriber: Successfully connected to SUB socket\n");

  /* Subscribe to group by filtering the received data*/
  rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "", 0);
  assert(rc == 0);
  g_print("Subscriber: Ready to receive data from broker %s\n",forwarder_address);

  g_free(forwarder_address);
  return subscriber;
}

gchar *receive_data(void *subscriber)
{
  zmq_msg_t message;
  zmq_msg_init (&message);
  if (zmq_recv (subscriber, &message, 0))
    return (NULL);
  gint size = zmq_msg_size (&message);
  gchar *data = malloc (size + 1);
  memcpy (data, zmq_msg_data (&message), size);
  zmq_msg_close (&message);
  data [size] = 0;

  return data;
}

void close_subscriber(void *subscriber)
{
  zmq_close (subscriber);
  //zmq_term (context);
}
