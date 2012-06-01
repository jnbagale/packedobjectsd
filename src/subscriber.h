
#include <glib.h>


typedef struct {
  void *context;
  void *subscriber;
  gchar *group_hash;
  gchar *user_hash;
  gchar *host;
  gint port;
  
} subObject;

subObject *make_sub_object(void);
void free_sub_object(subObject *sub_obj);
subObject *subscribe_forwarder(subObject *sub_obj);
void receive_data(subObject *sub_obj);
