
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

#include "config.h" 
#include "broker.h" /* for enum types: ENCODED/PLAIN and PUBLISHER/SUBSCRIBER */

typedef struct {
  void *context;
  void *subscriber;
  int port;
  int encode_type;
  char *message;
  char *address;
  char *sub_endpoint;
} subObject;

typedef struct {
  void *context;
  void *publisher;
  int port;
  char *address;
  char *pub_endpoint;
 } pubObject;

subObject *make_sub_object();
void *subscribe_to_broker(subObject *sub_obj, char *path_schema);
subObject *receive_data(subObject *sub_obj);
void unsubscribe_to_broker(subObject *sub_obj);
void free_sub_object(subObject *sub_obj);
pubObject *make_pub_object();
pubObject *publish_to_broker(pubObject *pub_obj, char *path_schema);
int send_data(pubObject *pub_obj, char *message, int message_length, int encode_type);
void unpublish_to_broker(pubObject *pub_obj);
void free_pub_object(pubObject *pub_obj);

#endif
/* End of packedobjectsd.h */
