// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

/* A ZeroMQ broker which receives messages from multiple publishers and forwards subscribers */
/* Binds subscribers to outbound socket */
/* Binds publishers to inbound socket */

#include <glib.h>
#include <stdlib.h> /* for exit()   */

#include "config.h"
#include "forwarder.h"

int main(int argc, char** argv)
{
  gint in_port = DEFAULT_IN_PORT;
  gint out_port = DEFAULT_OUT_PORT;
  gchar *address = DEFAULT_ADDRESS;
  gchar *hash_schema = NULL;
  gboolean verbose = FALSE;
  GError *error = NULL;
  GOptionContext *context;
  brokerObject *broker_obj = NULL; 

  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "zeromq broker adress", NULL },
    { "out_port", 's', 0, G_OPTION_ARG_INT, &out_port, "zeromq broker's outbound port: where subs connect", "N" },
    { "in_port", 'p', 0, G_OPTION_ARG_INT, &in_port, "zeromq broker's inbound port: where pubs connect", "N" },
    { NULL }
  };

  context = g_option_context_new ("- broker");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  
  /* Initialisation of broker object & variables */
  broker_obj = make_broker_object();
  broker_obj = init_broker( broker_obj, address, in_port, out_port);

  /* Send Broker details to the server*/
  connect_to_server(broker_obj, hash_schema);

  /* Start the broker */
  start_broker(broker_obj);

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
  
}
