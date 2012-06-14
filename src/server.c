// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <glib.h>
#include <stdlib.h>  /* for exit()   */

#include "config.h"
#include "lookup.h"

int main(int argc, char** argv)
{
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  gboolean verbose = FALSE;
  
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { NULL }
  };


  context = g_option_context_new ("- server");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  

  mainloop = g_main_loop_new(NULL, FALSE);  
  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }
  
  /* Run a thread to start the server */
  start_server();

  g_main_loop_run(mainloop);

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
}
