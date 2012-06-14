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

static gboolean publish_data(void *publisher)
{
  gchar *message = g_strdup_printf("%s", "test message");
  gint rc = send_data(publisher, message, strlen(message)); 
 
  if(rc !=0)
    {
      g_print("Publsiher failed to send data to broker");
    }
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
  gchar *broker = DEFAULT_BROKER;
  gint sub_port = DEFAULT_SUB_PORT;
  gint pub_port = DEFAULT_PUB_PORT;
  gboolean verbose = FALSE;
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  

  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "broker", 'h', 0, G_OPTION_ARG_STRING, &broker, "zeromq broker", NULL },
    { "sub_port", 's', 0, G_OPTION_ARG_INT, &sub_port, "broker's outbound port", "N" },
    { "pub_port", 'p', 0, G_OPTION_ARG_INT, &pub_port, "broker's inbound port", "N" },
    { "type",'t', 0, G_OPTION_ARG_STRING, &type, "node type:pub or sub or both", NULL },
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

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"pub") == 0) ) {
    /* Connects to PUB socket, program quits if connect fails * */
   
    void *publisher = publish_to_broker(broker, pub_port);
    g_timeout_add(1000, (GSourceFunc)publish_data, (gpointer)publisher);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"sub") == 0) ) {
    /* Connects to SUB socket, program quits if connect fails */
 
    void *subscriber = subscribe_to_broker(broker, sub_port);
    g_timeout_add(1000, (GSourceFunc)subscribe_data, (gpointer)subscriber);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;
}
