
#include <glib.h>

typedef struct {

  void *context;
  void *frontend;
  void *backend;
  gint out_port;
  gint in_port;
  gchar *address;
  gchar *front_endpoint;
  gchar *back_endpoint;
} brokerObject;

brokerObject *make_broker_object();
brokerObject *init_broker(brokerObject *broker_obj, gchar *address, gint in_port, gint out_port);
void start_broker(brokerObject *broker_obj);
void connect_to_server(brokerObject *broker_obj, gchar *hash_schema);
void free_broker_object(brokerObject *broker_obj);
