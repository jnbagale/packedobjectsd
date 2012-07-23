// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

#include <glib.h>
#include <stdlib.h>  /* for exit()   */
#include <db.h>

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
    printf("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  

  mainloop = g_main_loop_new(NULL, FALSE);  
  if (mainloop == NULL) {
    printf("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }

  /* Initialise the database */
  DB *db_ptr = NULL;
  db_ptr = init_bdb(db_ptr);
  db_ptr = write_db(db_ptr);
  read_db(db_ptr);
  close_bdb(db_ptr);
  /* Run a thread to start the server */
  //start_server();

  g_main_loop_run(mainloop);

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
}
