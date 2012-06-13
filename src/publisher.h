
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

gint send_data(void *publisher, gchar *message, gint msglen)
{
  gint rc;
  zmq_msg_t z_msg;
  msglen = 1000;

 // Send message to broker for group 
  rc = zmq_msg_init_size (&z_msg, msglen);
  assert(rc ==0);
  memcpy (zmq_msg_data (&z_msg), message, msglen);
  rc = zmq_send (publisher, &z_msg, 0);
  assert(rc == 0);

  g_print("Message sent: %s\n",message);
  zmq_msg_close (&z_msg);

  return rc;
}

void close_publisher(void *publisher)
{
  zmq_close (publisher);
  //zmq_term (context);
}
