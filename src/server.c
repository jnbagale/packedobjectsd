// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include "config.h"


int main(int argc, char** argv)
{
  GMainLoop *mainloop = NULL;
  GError *error = NULL;
  GOptionContext *context;
  gboolean verbose = FALSE;
   
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { NULL }
  };


  context = g_option_context_new ("- messaging framework");
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

  

  // g_timeout_add((scan_freq * 1000), (GSourceFunc)findDevices, (gpointer)bobj->dbusObject);

  g_main_loop_run(mainloop);
  
  return EXIT_FAILURE;  
  
}
