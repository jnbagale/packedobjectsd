
#ifndef PACKEDOBJECTSD_H_
#define PACKEDOBJECTSD_H_

#include <packedobjects/packedobjects.h>
/* Undefine conflicting macroses from config.h included in packedobjects.h
   and our config.h
*/
#undef VERSION
#undef PACKAGE
#undef PROGNAME
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
#undef PACKAGE_TARNAME
#undef PACKAGE_BUGREPORT

/* #ifdef HAVE_CONFIG_H */
/* #include "config.h" */
/* #endif */
#include "xmlutils.h"

#ifdef __QNX__
#undef POD_SCHEMA
#define POD_SCHEMA "app/native/packedobjectsd.xsd" // Using local path for pod schema in blackberry 10 QNX
#endif

#define DEFAULT_SERVER_ADDRESS "127.0.0.1" //"buildhost.uwl.ac.uk"   /* the default lookup server address */
#define DEFAULT_SERVER_PORT 5555  /* the default lookup server port */

enum ERR_CODE {
  INVALID_NODE_TYPE,
  INVALID_SCHEMA_FILE,
  SUBSCRIBE_FAILED,
  PUBLISH_FAILED,
  MALLOC_FAILED,
  INIT_PO_FAILED,
  RECEIVE_FAILED,
  DECODE_FAILED,
  SEND_FAILED,
  ENCODE_FAILED,
  UNDEFINED
};

enum NODE_TYPE {
  PUBLISHER,
  SUBSCRIBER,
  PUBSUB,
  SEARCHER,
  RESPONDER,
  SEARES
};

enum INIT_FLAGS {
  NO_COMPRESSION = 1,
  NO_HEARTBEAT = 2,
};

typedef struct {
  void *publisher_context;
  void *subscriber_context;
  void *publisher_socket;
  void *subscriber_socket;
  char *sub_topic;
  char *pub_topic;
  char uid_str[50];
  char *last_searcher_id;
  char *schema_hash;
  char *server_address;
  char *publisher_endpoint;
  char *subscriber_endpoint;
  int bytes_sent;
  int bytes_received;
  int error_code;
  int server_port;
  int node_type;
  int init_options;
  int heartbeat; // -1 when heartbeat is off, 1 for regular heartbeat and 0 when heartbeat isn't received
  double encode_cpu_time;
  double decode_cpu_time;
  unsigned long unique_id;
  unsigned long last_searcher;
  packedobjectsContext *pc;
} packedobjectsdObject;

// API functions for initialising and freeing
packedobjectsdObject *init_packedobjectsd(const char *schema_file, int node_type, int options);
void free_packedobjectsd(packedobjectsdObject *pod_obj);

// API functions for simple PUB SUB communication
int packedobjectsd_send(packedobjectsdObject *pod_obj, xmlDocPtr doc);
xmlDocPtr packedobjectsd_receive(packedobjectsdObject *pod_obj);

// API functions for SEARCHER RESPONDER communication
int packedobjectsd_send_search(packedobjectsdObject *pod_obj, xmlDocPtr doc);
xmlDocPtr packedobjectsd_receive_search(packedobjectsdObject *pod_obj);
int packedobjectsd_send_response(packedobjectsdObject *pod_obj, xmlDocPtr doc);
xmlDocPtr packedobjectsd_receive_response(packedobjectsdObject *pod_obj);

// API functions for error handling
const char *pod_strerror(int error_code);
#endif
/* End of packedobjectsd.h */
