
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef LOOKUP_H_
#define LOOKUP_H_

#include <db.h>
#include "packedobjectsd.h"
 
typedef struct {
  DB *db_ptr;
  void *context;
  void *responder;
  void *requester;
  int port;
  char *address;
}serverObject;

serverObject *make_server_object (void);
serverObject *init_bdb(serverObject *server_obj);
serverObject *write_db(serverObject *server_obj,char *hash_schema);
int read_db(serverObject *server_obj, char *hash_schema, char *buffer);
serverObject *remove_db(serverObject *server_obj, char *hash_schema);
serverObject *close_bdb(serverObject *server_obj);
void *start_server(void *server_object);
void free_server_object(serverObject *server_obj);

#endif
/* End of lookup.h */
