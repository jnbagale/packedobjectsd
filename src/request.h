
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef BROKER_H_
#define BROKER_H_

#include "message.h"

#define MAX_BUFFER_SIZE 100 /* the maximum size for address buffer */
#define MAX_HASH_SIZE 128 /* the maximum size for hash of the schema */

typedef struct {
  char node_type;
  char *hash_schema;  
} Request;

static inline char *which_node (char node_type) {
  if(node_type == 'P') return "PUBLISHER";
  else if(node_type == 'S') return "SUBSCRIBER";
  else return NULL;
}

Request *make_request_object(); 
void free_request_object(Request *req); 
int serialize_request(char *buffer, Request *req); 
int deserialize_request(char *buffer, Request *req);
char *get_broker_detail(char node_type, char *address, int port, char *hash_schema);

#endif
/* End of request.h */