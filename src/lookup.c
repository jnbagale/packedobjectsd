#include <glib.h>

#include "config.h"

void start_server(void)
{
  // dummy function 
  g_print("I do nothing at the moment. I am just a dummy function\n");
}

gint store_address(gchar *schema_hash, gchar *address)
{
  //dummy function to retrieve and store store schema and address of broker
  return 1;
}

gchar *lookup_address(gchar *schema_hash)
{
  //dummy function to listen to look up request from nodes and return address of broker
  return "127.0.0.1";
}
