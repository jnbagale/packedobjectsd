
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <glib.h>

typedef struct {

  void *context;
  void *frontend;
  void *backend;
  gint out_port;
  gint in_port;
  gchar *address;
  gchar *front_endpoint;
  gchar *back_endpoint;
} brokerObject;

brokerObject *make_broker_object();
brokerObject *init_broker(brokerObject *broker_obj, gchar *address, gint in_port, gint out_port);
void start_broker(brokerObject *broker_obj);
void connect_to_server(brokerObject *broker_obj, gchar *hash_schema);
void free_broker_object(brokerObject *broker_obj);
