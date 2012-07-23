
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

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
  serverObject *server_obj = NULL;
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

  server_obj = make_server_object();
  start_server(server_obj);

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
