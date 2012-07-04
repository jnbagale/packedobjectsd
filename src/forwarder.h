
#include <glib.h>

typedef struct {

  void *context;
  void *frontend;
  void *backend;
  gint out_port;
  gint in_port;
  gchar *address;
  gchar *front_address;
  gchar *back_address;
} brokerObject;

brokerObject *make_broker_object(gchar *address, gint in_port, gint out_port);
void free_broker_object(brokerObject *broker_obj);
void start_broker(brokerObject *broker_obj);
void connect_to_server(gchar *address, gint in_port, gint out_port, gchar *hash_schema);
