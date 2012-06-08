
#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */

#include "config.h"
#include "publisher.h"

pubObject *make_pub_object(void)
{
  pubObject *pub_obj;

  if ((pub_obj = (pubObject *)g_malloc(sizeof(pubObject))) == NULL) {
    //g_printerr("failed to malloc pubObject!");
    exit(EXIT_FAILURE);
  }

  pub_obj->publish = TRUE;
  return pub_obj;
}

pubObject *publish_forwarder(pubObject *pub_obj)
{
  gint rc; 
  gint limit = 5;
  gchar *forwarder_address =  g_strdup_printf("tcp://%s:%d",pub_obj->host, pub_obj->port);

  /* Prepare our context and publisher */
  pub_obj->context = zmq_init (1);
  pub_obj->publisher = zmq_socket (pub_obj->context, ZMQ_PUB); 
  zmq_setsockopt (pub_obj->publisher, ZMQ_HWM, &limit, (sizeof limit));
  rc = zmq_connect (pub_obj->publisher, forwarder_address);
  assert(rc == 0);
  g_print("Publisher: Successfully connected to PUB socket\n");
  g_print("Publisher: Sending data to broker at %s\n",forwarder_address);
  g_free(forwarder_address);
  return pub_obj;
}

static gint z_send(void *socket ,gchar *string)
{
  gint rc;
  zmq_msg_t message;
  zmq_msg_init_size (&message, strlen (string));
  memcpy (zmq_msg_data (&message), string, strlen (string));
  rc = zmq_send (socket, &message, 0);
  zmq_msg_close (&message);
  return (rc);
}

void send_data(pubObject *pub_obj)
{
  gint counter = 0;
  while(1)
    {
      // Send message to all subscribers of group: world
      gchar *update;
      update = g_strdup_printf("%s %s %d",pub_obj->group_hash, pub_obj->user_hash, counter);
      z_send (pub_obj->publisher, update); 
      g_print("Sent :%s\n",update);
      g_free(update);
      g_usleep(1000000); // 1 message per second
      counter++;
    }
}

void free_pub_object(pubObject *pub_obj)
{
  zmq_close (pub_obj->publisher);
  zmq_term (pub_obj->context);
  g_free(pub_obj->host);
  g_free(pub_obj);  
}
