
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

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

#define DEFAULT_SERVER_ADDRESS "ec2-107-20-219-103.compute-1.amazonaws.com" /* the default lookup server address */
#define DEFAULT_SERVER_PORT 5555  /* the default lookup server port */

enum ERROR_CODE {
  INVALID_NODE_TYPE,
  INVALID_SCHEMA_FILE,
  SUBSCRIBE_FAILED,
  PUBLISH_FAILED,
  MALLOC_FAILED,
  INIT_PO_FAILED,
  RECEIVE_FAILED,
  DECODE_FAILED,
  SEND_FAILED,
  ENCODE_FAILED
};

typedef struct {
  void *publisher_context;
  void *subscriber_context;
  void *publisher_socket;
  void *subscriber_socket;
  char *schema_hash;
  char *server_address;
  char *publisher_endpoint;
  char *subscriber_endpoint;
  char node_type;    /* Subscriber 'S'; Publisher 'P'; Both 'B' */
  int bytes_sent;
  int bytes_received;
  int error_code;
  int server_port;
  packedobjectsContext *pc;
} packedobjectsdObject;

packedobjectsdObject *init_packedobjectsd(const char *schema_file);
int packedobjectsd_send(packedobjectsdObject *pod_obj, xmlDocPtr doc);
xmlDocPtr packedobjectsd_receive(packedobjectsdObject *pod_obj);
void free_packedobjectsd(packedobjectsdObject *pod_obj);
const char *pod_strerror(int error_code);
#endif
/* End of packedobjectsd.h */
