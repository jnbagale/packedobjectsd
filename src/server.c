
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <db.h>
#include <glib.h>
#include <pthread.h> /* for threads */
#include <stdlib.h>  /* for exit()   */


#include "config.h"
#include "lookup.h"

int main(int argc, char** argv)
{
  char *address = DEFAULT_ADDRESS;
  pthread_t thread_server;
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  gboolean verbose = FALSE;
  serverObject *server_obj = NULL;
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "Lookup server address", NULL },
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

  /* Create new server object */
  server_obj = make_server_object();
  /* Allocate memory for address pointer */
  server_obj->address = malloc(strlen(address) + 1);
  sprintf(server_obj->address, "%s",address);

  /* Create thread which will execute start_server() function */
  if (pthread_create( &thread_server, NULL, start_server,(void *) server_obj)) {
    fprintf(stderr, "Error creating thread \n");
    exit(EXIT_FAILURE);
  }

  /* Join the thread to start the server */
  if(pthread_join( thread_server, NULL)) {
    fprintf(stderr, "Error joining thread \n");
    exit(EXIT_FAILURE);
  }


 
  g_main_loop_run(mainloop);

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
}
