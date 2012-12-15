
#ifndef BROKER_H_
#define BROKER_H_

#include "message.h"
#include "xmlutils.h"

#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

char *get_broker_detail(char node_type, char *address, int port, char *schema_hash);

#endif
/* End of broker.h */
