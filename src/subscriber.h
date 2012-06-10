
#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */

void *subscribe_to_broker(gchar *broker_address, gint broker_sub_port, gchar *group_hash)
{
  gint rc;
  void *subscriber;
  void *context = zmq_init (1);
  gchar *filter =   g_strdup_printf("%s", group_hash);
  gchar *forwarder_address =  g_strdup_printf("tcp://%s:%d",broker_address, broker_sub_port);
  
   /* Socket to subscribe to broker */
  subscriber = zmq_socket (context, ZMQ_SUB);
  rc = zmq_connect (subscriber, forwarder_address);
  assert(rc == 0);
  g_print("Subscriber: Successfully connected to SUB socket\n");

  /* Subscribe to group by filtering the received data*/
  zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, filter  , strlen(filter));
  g_print("Subscriber: Ready to receive data from broker %s for group %s \n",forwarder_address, filter);

  g_free(filter);
  g_free(forwarder_address);
  return subscriber;
}

static char * z_receive(void *socket)
{
  zmq_msg_t message;
  zmq_msg_init (&message);
  if (zmq_recv (socket, &message, 0))
    return (NULL);
  gint size = zmq_msg_size (&message);
  gchar *data = malloc (size + 1);
  memcpy (data, zmq_msg_data (&message), size);
  zmq_msg_close (&message);
  data [size] = 0;

  return data;
}

gchar *receive_data(void *subscriber)
{
  gchar *data = z_receive (subscriber);
  return data;
}

void close_subscriber(void *subscriber)
{
  zmq_close (subscriber);
  //zmq_term (context);
}
