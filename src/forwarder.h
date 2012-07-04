
#include <glib.h>

typedef struct {

  void *context;
  void *frontend;
  void *backend;
  gint out_port;
  gint in_port;
  gchar *broker;
} brokerObject;

brokerObject *make_broker_object(void);
void free_broker_object(brokerObject *broker_obj);
void start_broker(brokerObject *broker_obj);
