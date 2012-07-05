// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

/* A sample ZeroMQ node which can act as both publisher and subscriber */
/* Subscriber connects to broker's outbound socket */
/* Publisher connects to broker's inbound socket */

#include <zmq.h>
#include <glib.h>
#include <string.h>  /* for strlen() */
#include <stdlib.h>  /* for exit()   */

#include "config.h"
#include "publisher.h"
#include "subscriber.h"

static gint global_send_counter = 0;

static gboolean publish_data(pubObject *pub_obj)
{
  gchar *message = g_strdup_printf("%s#%d", "test message",global_send_counter++);
  send_data(pub_obj, message, strlen(message)); 
  g_print("Message sent: %s\n", message);

  g_free(message);
  return TRUE;  
}

static gboolean subscribe_data(void *subscriber)
{
  gchar *message = receive_data(subscriber);
  g_print("Message received: %s \n", message);

  g_free(message);
  return TRUE;  
}

int main (int argc, char *argv [])
{
  gchar *type = DEFAULT_TYPE;
  gchar *address = DEFAULT_ADDRESS;
  gint out_port = DEFAULT_OUT_PORT;
  gint in_port = DEFAULT_IN_PORT;
  gint recv_freq = DEFAULT_RECV_FREQ;
  gint send_freq = DEFAULT_SEND_FREQ;
  gboolean verbose = FALSE;
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  pubObject *pub_obj = NULL;

  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "zeromq broker address", NULL },
    { "type",'t', 0, G_OPTION_ARG_STRING, &type, "node type:pub or sub or both", NULL },
    { "out_port", 'i', 0, G_OPTION_ARG_INT, &out_port, "broker's outbound port: where subs connect", "N" },
    { "in_port", 'o', 0, G_OPTION_ARG_INT, &in_port, "broker's inbound port: where pubs connect", "N" },
    { "recv_freq", 'r', 0, G_OPTION_ARG_INT, &recv_freq, "Receiving frequency for subscriber", "N" },
    { "send_freq", 's', 0, G_OPTION_ARG_INT, &send_freq, "Sending frequency for publisher", "N" },
    { NULL }
  };
 
  context = g_option_context_new ("- node");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }
   
  /* Initialise mainloop */
  mainloop = g_main_loop_new(NULL, FALSE);
  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }

  pub_obj = make_pub_object();  
  
  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"pub") == 0) ) {
    /* Connects to PUB socket, program quits if connect fails * */
   
    pub_obj = publish_to_broker(pub_obj, address, in_port);
    g_timeout_add(send_freq, (GSourceFunc)publish_data, (gpointer)pub_obj);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"sub") == 0) ) {
    /* Connects to SUB socket, program quits if connect fails */
 
    void *subscriber = subscribe_to_broker(address, out_port);
    g_timeout_add(recv_freq, (GSourceFunc)subscribe_data, (gpointer)subscriber);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;
}
