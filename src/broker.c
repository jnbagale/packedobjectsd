// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <uuid/uuid.h>

#include "config.h"
#include "forwarder.h"

int main(int argc, char** argv)
{

  uuid_t buf;
  gchar id[36];
  gchar *user_hash;
  gchar *group_hash;
  gchar *group = DEFAULT_GROUP;
  gchar *host = DEFAULT_HOST;
  gint port = DEFAULT_PORT;
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
    { "port", 'p', 0, G_OPTION_ARG_INT, &port, "zeromq port", "N" },
    { NULL }
  };


  context = g_option_context_new ("- broker");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  

  broker_obj = make_broker_object();
  broker_obj->host =  g_strdup_printf("%s",host);
  broker_obj->port = port;

   mainloop = g_main_loop_new(NULL, FALSE);  
  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }

    /* Run a thread to start the forwarder */
  if( g_thread_create( (GThreadFunc) start_forwarder, (gpointer) broker_obj, FALSE, &error) == NULL ) {
       g_printerr("option parsing failed 2: %s\n", error->message);
   exit (EXIT_FAILURE);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
  
}
