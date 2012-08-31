
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A ZeroMQ broker which receives messages from multiple publishers and forwards subscribers */
/* Binds subscribers to outbound socket */
/* Binds publishers to inbound socket */

#ifndef BROKER_H_
#define BROKER_H_

#include "message.h"

#define MAX_BUFFER_SIZE 100 /* the maximum size for address buffer */

static inline char *which_node (int node_type) {
  return ((node_type) ? "PUBLISHER" : "SUBSCRIBER"); 
}

char *get_broker_detail(int node_type, char *address, int port, char *hash_schema);

#endif
/* End of broker.h */
