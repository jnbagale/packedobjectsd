
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <db.h>

typedef struct {
  DB *db_ptr;
  void *context;
  void *responder;
  void *requester;
  int port;
  char *address;
} serverObject;

serverObject *make_server_object (void);
void *start_server(void *server_object);
void free_server_object(serverObject *server_obj);
