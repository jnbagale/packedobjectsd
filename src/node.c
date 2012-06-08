/* A ZeroMQ node who acts as both publisher and subscriber */
/* Connects SUB socket to broker's out socket address */
/* Connects PUB socket to broker's in socket address */

#include <zmq.h>
#include <glib.h>
#include <string.h>  /* for strlen() */
#include <stdlib.h>  /* for exit()   */
#include <uuid/uuid.h>
#include <glib/gthread.h>

#include "config.h"
#include "publisher.h"
#include "subscriber.h"



int main (int argc, char *argv [])
{
  GMainLoop *mainloop;
  GError *error;
  uuid_t buf;
  gchar id[36];
  gchar *user_hash;
  gchar *group_hash;
  gchar *group = DEFAULT_GROUP;
  gchar *host = DEFAULT_HOST;
  gchar *type = DEFAULT_TYPE;
  gint sub_port = DEFAULT_SUB_PORT;
  gint pub_port = DEFAULT_PUB_PORT;
  gboolean verbose = FALSE;
  GOptionContext *context;
  subObject *sub_obj = NULL;  
  pubObject *pub_obj = NULL;


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

  sub_obj = make_sub_object();
  pub_obj = make_pub_object();

  sub_obj->port = sub_port;
  pub_obj->port = pub_port;
  sub_obj->host =  g_strdup_printf("%s",host);
  pub_obj->host =  g_strdup_printf("%s",host);

  /* Initialising thread */
  g_thread_init(NULL);
  
  uuid_generate_random(buf);
  uuid_unparse(buf, id);
  /* generate a hash of a unique id */
  user_hash = g_compute_checksum_for_string(G_CHECKSUM_MD5, id, strlen(id));

  /* generate a hash of the group name */
  group_hash = g_compute_checksum_for_string(G_CHECKSUM_MD5, group, strlen(group));
   
  sub_obj->group_hash = g_strdup_printf("%s", group_hash);
  sub_obj->user_hash =  g_strdup_printf("%s", user_hash);
  pub_obj->group_hash = g_strdup_printf("%s", group_hash);
  pub_obj->user_hash =  g_strdup_printf("%s", user_hash);

  g_free(user_hash);
  g_free(group_hash);
  
  /* Initialise mainloop */
  mainloop = g_main_loop_new(NULL, FALSE);

  if (mainloop == NULL) {
    g_printerr("Couldn't create GMainLoop\n");
    exit(EXIT_FAILURE);
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"sub") == 0) ) {
    /* Connects to SUB socket, program quits if connect fails */
    sub_obj = subscribe_forwarder(sub_obj);
    if( g_thread_create( (GThreadFunc) receive_data, (gpointer) sub_obj, FALSE, &error) == NULL) {
      g_printerr("option parsing failed1: %s\n", error->message);
      exit (EXIT_FAILURE);
    }
  }

  if( (g_strcmp0(type,"both") == 0) || (g_strcmp0(type,"pub") == 0) ) {
    /* Connects to PUB socket, program quits if connect fails * */
    pub_obj = publish_forwarder(pub_obj);
    if( g_thread_create( (GThreadFunc) send_data, (gpointer) pub_obj, FALSE, &error) == NULL ) {
      g_printerr("option parsing failed 2: %s\n", error->message);
      exit (EXIT_FAILURE);
    }
  }

  g_main_loop_run(mainloop);
  
  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;
}
