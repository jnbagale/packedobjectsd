#include <glib.h>

void start_server(void);
gint store_address(gchar *schema_hash, gchar *address);
gchar *lookup_address(gchar *schema_hash);
