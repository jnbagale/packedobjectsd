/* A ZeroMQ node who acts as both publisher and subscriber */
/* Connects SUB socket to broker's out socket address */
/* Connects PUB socket to broker's in socket address */

#include <zmq.h>
#include <glib.h>
#include <stdio.h>   /* for sscanf() */
#include <string.h>  /* for strlen() */
#include <stdlib.h>  /* for exit()   */
#include <uuid/uuid.h>

#include "config.h"
#include "publisher.h"
#include "subscriber.h"

int main (int argc, char *argv [])
{
  GMainLoop *mainloop;
  GError *error;
  uuid_t buf;
  gchar id[36];
  gchar *sender_hash;
  gchar *group_hash;
  gchar *group = DEFAULT_GROUP;
  gchar *host = DEFAULT_HOST;
  gchar *type = DEFAULT_TYPE;
  gint sub_port = DEFAULT_SUB_PORT;
  gint pub_port = DEFAULT_PUB_PORT;
  gboolean verbose = FALSE;
  GOptionContext *context;
  
  GOptionEntry entries[] = 
  {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
    { "group", 'g', 0, G_OPTION_ARG_STRING, &group, "zeromq group", NULL },
    { "host", 'h', 0, G_OPTION_ARG_STRING, &host, "zeromq host", NULL },
    { "sub_port", 's', 0, G_OPTION_ARG_INT, &sub_port, "broker's outbound port", "N" },
    { "pub_port", 'p', 0, G_OPTION_ARG_INT, &pub_port, "broker's inbound port", "N" },
    { "type",'t', 0, G_OPTION_ARG_STRING, &type, "node type:pub or sub or both", NULL },
    { NULL }
  };
 
  context = g_option_context_new ("- node");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }

  uuid_generate_random(buf);
  uuid_unparse(buf, id);
  /* generate a hash of a unique id */
  sender_hash = g_compute_checksum_for_string(G_CHECKSUM_MD5, id, strlen(id));

  /* generate a hash of the group name */
  group_hash = g_compute_checksum_for_string(G_CHECKSUM_MD5, group, strlen(group));
   
  /* Initialise mainloop */
  mainloop = g_main_loop_new(NULL, FALSE);

  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"pub") == 0) ) {
    /* Connects to PUB socket, program quits if connect fails * */
    gchar *message = g_strdup_printf("%s", "test message");
    void *publisher = publish_to_broker(host, pub_port);
    gint rc = send_data(publisher, group_hash, sender_hash, message); 
    if(rc !=0) g_print("Publsiher failed to send data to broker");
    g_free(message);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"sub") == 0) ) {
    /* Connects to SUB socket, program quits if connect fails */
    gchar group[32] ;
    gchar sender[32];
    gchar message[100];
    void *subscriber = subscribe_to_broker(host, sub_port, group_hash);
    gchar *data = receive_data(subscriber);
    sscanf (data, "%s %s %s", group, sender, message);
    g_print("Received message %s from sender %s of group %s\n", message, sender, group);
    g_free(data);
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  g_free(sender_hash);
  g_free(group_hash);
  return EXIT_FAILURE;
}
