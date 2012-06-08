// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <glib.h>
#include <stdlib.h> /* for exit()   */
#include <glib/gthread.h>

#include "config.h"
#include "forwarder.h"

int main(int argc, char** argv)
{
  gchar *group = DEFAULT_GROUP;
  gchar *host = DEFAULT_HOST;
  gint sub_port = DEFAULT_SUB_PORT;
  gint pub_port = DEFAULT_PUB_PORT;
  gboolean verbose = FALSE;
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  brokerObject *broker_obj = NULL; 

  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "group", 'g', 0, G_OPTION_ARG_STRING, &group, "zeromq group", NULL },
    { "host", 'h', 0, G_OPTION_ARG_STRING, &host, "zeromq host", NULL },
    { "sub_port", 's', 0, G_OPTION_ARG_INT, &sub_port, "zeromq broker's outbound port", "N" },
    { "pub_port", 'p', 0, G_OPTION_ARG_INT, &pub_port, "zeromq broker's inbound port", "N" },
    { NULL }
  };


  context = g_option_context_new ("- broker");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  

  /* Initialising thread */
  g_thread_init(NULL);
  
  broker_obj = make_broker_object();
  broker_obj->sub_port = sub_port;
  broker_obj->pub_port = pub_port;
  broker_obj->host =  g_strdup_printf("%s",host);
  broker_obj->group =  g_strdup_printf("%s",group);

  /* Initialising mainloop */
  mainloop = g_main_loop_new(NULL, FALSE);  
  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }
  
  // networking code to connect to server and
  // send hash of schema and network address will come here
  
  /* Run a thread to start the broker */
  if( g_thread_create( (GThreadFunc) start_forwarder, (gpointer) broker_obj, FALSE, &error) == NULL ) {
       g_printerr("option parsing failed 2: %s\n", error->message);
   exit (EXIT_FAILURE);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
  
}
