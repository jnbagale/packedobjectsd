/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A sample ZeroMQ node which can act as both publisher and subscriber */
/* Subscriber connects to broker's outbound socket */
/* Publisher connects to broker's inbound socket */

#include <zmq.h>
#include <glib.h>
#include <string.h>  /* for strlen() */
#include <stdlib.h>  /* for exit()   */
#include <unistd.h>  /* for read */
#include <fcntl.h>   /* for open() */

#include "config.h"
#include "publisher.h"
#include "subscriber.h"

static int global_send_counter = 0;

static gboolean publish_data(pubObject *pub_obj)
{
  gchar *message = g_strdup_printf("%s#%d", "test message",global_send_counter++);
  send_data(pub_obj, message, strlen(message), "1"); 
  g_print("Message sent: %s\n", message);

  g_free(message);
  return TRUE;  
}

static gboolean subscribe_data(subObject *sub_obj)
{
  gchar *message = receive_data(sub_obj);
  g_print("Message received: %s \n", message);

  g_free(message);
  return TRUE;  
}

int main (int argc, char *argv [])
{
  char *type = DEFAULT_TYPE;
  char *address = DEFAULT_ADDRESS;
  int out_port = DEFAULT_OUT_PORT;
  int in_port = DEFAULT_IN_PORT;
  int recv_freq = DEFAULT_RECV_FREQ;
  int send_freq = DEFAULT_SEND_FREQ;
  gboolean verbose = FALSE;
  GError *error = NULL;
  GOptionContext *context;
  GMainLoop *mainloop = NULL;
  pubObject *pub_obj = NULL;
  subObject *sub_obj= NULL;
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "zeromq broker address", NULL },
    { "type",'t', 0, G_OPTION_ARG_STRING, &type, "node type:pub or sub or both", NULL },
    { "out_port", 'i', 0, G_OPTION_ARG_INT, &out_port, "broker's outbound port: where subs connect", "N" },
    { "in_port", 'o', 0, G_OPTION_ARG_INT, &in_port, "broker's inbound port: where pubs connect", "N" },
    { "recv_freq", 'r', 0, G_OPTION_ARG_INT, &recv_freq, "Receiving frequency for subscriber", "N" },
    { "send_freq", 's', 0, G_OPTION_ARG_INT, &send_freq, "Sending frequency for publisher", "N" },
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

  
  /* Initialise objects & variables */
  pub_obj = make_pub_object();  
  sub_obj = make_sub_object();
  
  pub_obj->in_port = in_port;
  pub_obj->address = g_strdup_printf("%s",address);
  sub_obj->out_port = out_port;
  sub_obj->address = g_strdup_printf("%s",address);
  
  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"pub") == 0) ) {
    /* Connects to PUB socket, program quits if connect fails * */
   
    pub_obj = publish_to_broker(pub_obj);
 
    g_timeout_add(send_freq, (GSourceFunc)publish_data, (gpointer)pub_obj);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"sub") == 0) ) {
    /* Connects to SUB socket, program quits if connect fails */
 
    sub_obj = subscribe_to_broker(sub_obj);
    g_timeout_add(recv_freq, (GSourceFunc)subscribe_data, (gpointer)sub_obj);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;
}
