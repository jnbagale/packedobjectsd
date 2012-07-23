
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

typedef struct {

  void *context;
  void *frontend;
  void *backend;
  int out_port;
  int in_port;
  char *address;
  char *front_endpoint;
  char *back_endpoint;
} brokerObject;

brokerObject *make_broker_object();
brokerObject *init_broker(brokerObject *broker_obj, char *address, int in_port, int out_port);
void start_broker(brokerObject *broker_obj);
void connect_to_server(brokerObject *broker_obj, char *hash_schema);
void free_broker_object(brokerObject *broker_obj);
