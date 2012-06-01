

#include <glib.h>


typedef struct {

  void *context;
  void *frontend;
  void *backend;
  gchar *group_hash;
  gchar *user_hash;
  gchar *host;
  gint port;
} brokerObject;

brokerObject *make_broker_object(void);
void free_broker_object(brokerObject *broker_obj);
void start_forwarder(brokerObject *broker_obj);
