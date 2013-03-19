
#ifndef BROKER_H_
#define BROKER_H_

#include "message.h"
#include "xmlutils.h"
#include "packedobjectsd.h"

#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

int get_broker_detail(packedobjectsdObject *pod_obj);

#endif
/* End of broker.h */
