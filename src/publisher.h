
#include <glib.h>


typedef struct {
  void *context;
  void *publisher;
  gchar *group_hash;
  gchar *user_hash;
  gboolean publish;
  gchar *host;
  gint port;
  
} pubObject;

pubObject *make_pub_object(void);
void free_pub_object(pubObject *pub_obj);
pubObject *publish_forwarder(pubObject *pub_obj);
void send_data(pubObject *pub_obj);
