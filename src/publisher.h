
#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */


void *publish_to_broker(gchar *broker_address, gint broker_pub_port)
{
  gint rc; 
  void *publisher;
  void *context = zmq_init (1);
  gchar *forwarder_address =  g_strdup_printf("tcp://%s:%d",broker_address, broker_pub_port);

  /* Prepare our context and publisher */
  publisher = zmq_socket (context, ZMQ_PUB); 
  rc = zmq_connect (publisher, forwarder_address);
  assert(rc == 0);
  g_print("Publisher: Successfully connected to PUB socket\n");
  g_print("Publisher: Ready to send data to broker at %s\n",forwarder_address);
  g_free(forwarder_address);
  return publisher;
}

static gint z_send(void *socket, gchar *data)
{
  gint rc;
  zmq_msg_t message;
  zmq_msg_init_size (&message, strlen (data));
  memcpy (zmq_msg_data (&message), data, strlen (data));
  rc = zmq_send (socket, &message, 0);
  zmq_msg_close (&message);
  return rc;
}

gint send_data(void *publisher, gchar *group_hash, gchar *sender_hash, gchar *message)
{
  gint rc;
  gchar *data = g_strdup_printf("%s %s %s",group_hash, sender_hash, message);
  // Send message to broker for group 
  rc = z_send (publisher, data); 
  g_print("Sent :%s\n",data);
  g_free(data);

  return rc;
}

void close_publisher(void *publisher)
{
  zmq_close (publisher);
  //zmq_term (pub_obj->context);
}
