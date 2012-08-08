
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A ZeroMQ broker which receives messages from multiple publishers and forwards subscribers */
/* Binds subscribers to outbound socket */
/* Binds publishers to inbound socket */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h> /* for exit()   */

#include "config.h"
#include "broker.h"

int main(int argc, char** argv)
{
  int port_in = DEFAULT_PORT_IN;
  int port_out = DEFAULT_PORT_OUT;
  char *address = DEFAULT_SERVER_ADDRESS;
  gboolean verbose = FALSE;
  brokerObject *broker_obj = NULL; 
  /* For command line arguments */
  GError *error = NULL;
  GOptionContext *context;
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "zeromq broker address", NULL },
    { "port_out", 'o', 0, G_OPTION_ARG_INT, &port_out, "zeromq broker outbound port: where subs connect", "N" },
    { "port_in", 'i', 0, G_OPTION_ARG_INT, &port_in, "zeromq broker inbound port: where pubs connect", "N" },
    { NULL }
  };

  context = g_option_context_new ("- broker");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    printf("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }  
  /* Initialisation of broker object & variables */
  broker_obj = make_broker_object();
  broker_obj = init_broker( broker_obj, address, port_in, port_out);

  /* Start the broker */
  start_broker(broker_obj);

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;  
  
}
/* End of broker.c */
