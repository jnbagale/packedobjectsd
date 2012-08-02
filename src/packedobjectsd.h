
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

enum ENCODE_TYPE {ENCODED, PLAIN};      /* Supported message encoding types */
enum NODE_TYPE {PUBLISHER, SUBSCRIBER}; /* Supported node types */

static inline char *which_node (int node_type) {
  return ((node_type) ? "SUBSCRIBER" : "PUBLISHER"); 
}

typedef struct {
  unsigned int port_in;
  unsigned int port_out;
  char *address;  
} Address;

Address *make_address_object(void); 
Address *create_address(Address *addr, char *address, int port_in, int port_out); 
void free_address_object(Address *addr); 
int serialize_address(char *buffer, Address *addr);
int deserialize_address(char *buffer, Address *addr);
int send_message(void *socket, char *message, int message_length); 
int send_message_more(void *socket, char *message, int message_length); 
char *receive_message(void *socket); 
char *receive_message_more(void *socket);
char *get_broker_detail(int node_type, char *address, int port, char *path_schema);

#endif
/* End of packedobjectsd.h */
