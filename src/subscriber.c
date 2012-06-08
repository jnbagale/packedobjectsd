
#include <zmq.h>
#include <glib.h>
#include <assert.h> /* for assert() */
#include <stdio.h>  /* for sscanf() */
#include <string.h> /* for strlen() */
#include <stdlib.h> /* for exit()   */

#include "config.h"
#include "subscriber.h"


subObject *make_sub_object(void)
{
  subObject *sub_obj;

  if ((sub_obj = (subObject *)g_malloc(sizeof(subObject))) == NULL) {
    //g_printerr("failed to malloc subObject!");
    exit(EXIT_FAILURE);
  }

  return sub_obj;
}

subObject *subscribe_forwarder(subObject *sub_obj)
{
  gint rc;
  gchar *forwarder_address =  g_strdup_printf("tcp://%s:%d",sub_obj->host, sub_obj->port);
  sub_obj->context = zmq_init (1);

   /* Socket to subscribe to forwarder */
  sub_obj->subscriber = zmq_socket (sub_obj->context, ZMQ_SUB);
  rc = zmq_connect (sub_obj->subscriber, forwarder_address);
  assert(rc == 0);
  g_print("Subscriber: Successfully connected to SUB socket\n");
  /* Subscribe to default group: world */
  gchar *filter =   g_strdup_printf("%s", sub_obj->group_hash);
  zmq_setsockopt (sub_obj->subscriber, ZMQ_SUBSCRIBE, filter  , strlen(filter));
  g_print("Subscriber: Receiving data from broker %s for group %s \n",forwarder_address, filter);

  g_free(filter);
  g_free(forwarder_address);
  return sub_obj;
}

static char * z_receive(void *socket)
{
  zmq_msg_t message;
  zmq_msg_init (&message);
  if (zmq_recv (socket, &message, 0))
    return (NULL);
  gint size = zmq_msg_size (&message);
  gchar *string = malloc (size + 1);
  memcpy (string, zmq_msg_data (&message), size);
  zmq_msg_close (&message);
  string [size] = 0;

  return (string);
}

void receive_data(subObject *sub_obj)
{
  while(1)
    {
      gchar *string = z_receive (sub_obj->subscriber);
      gchar message[50];
      gchar user_hash[75];
      gchar group_hash[75];
   
      sscanf (string, "%s %s %s",  group_hash, user_hash, message);
      g_print("Received message %s from user %s of group %s\n", message, user_hash, group_hash);
      
      g_free (string);
      g_usleep(1000000);//reads 1 message per second
    }
}

void free_sub_object(subObject *sub_obj)
{
  zmq_close (sub_obj->subscriber);
  zmq_term (sub_obj->context);
  g_free(sub_obj->host);
  g_free(sub_obj);  
}
