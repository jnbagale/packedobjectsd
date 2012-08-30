
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

#include <packedobjects.h>

#define DEFAULT_SERVER_ADDRESS "127.0.0.1" /* the default lookup server address */
#define DEFAULT_SERVER_PORT 5555  /* the default lookup server port */

enum ENCODE_TYPE {PLAIN, ENCODED};      /* Supported message encoding types */
enum NODE_TYPE {SUBSCRIBER, PUBLISHER, BOTH }; /* Supported node types */

typedef struct {
  void *publisher_context;
  void *subscriber_context;
  void *publisher_socket;
  void *subscriber_socket;
  char *path_schema;
  char *data_received;
  char *server_address;
  char *publisher_endpoint;
  char *subscriber_endpoint;
  int node_type;    /* Subscriber 0; Publisher 1; Both 2 */
  int encode_type; /* Plain 0; Encoded 1 */
  int server_port;
  packedobjectsContext *pc;
} packedobjectsdObject;


packedobjectsdObject *packedobjectsd_init(int node_type, char *path_schema, char *server_address, int server_port);
xmlDocPtr receive_data(packedobjectsdObject *pod_obj);
int send_data(packedobjectsdObject *pod_obj, xmlDocPtr doc, int encode_type);
void packedobjectsd_free(packedobjectsdObject *pod_obj);

#endif
/* End of packedobjectsd.h */
