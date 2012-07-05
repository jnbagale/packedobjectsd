
#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */
#include <inttypes.h> /* for uint64_t */

typedef struct {
  void *context;
  void *publisher;
  gint in_port;
  gchar *address;
  gchar *pub_endpoint;
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
  gint rc; 
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

gint send_data(pubObject *pub_obj, gchar *message, gint msglen)
{
  gint rc;
  zmq_msg_t z_msg;
  
  // Send message to broker for group 
  rc = zmq_msg_init_size (&z_msg, msglen);
  assert(rc ==0);
  memcpy (zmq_msg_data (&z_msg), message, msglen);
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
