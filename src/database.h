
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <db.h>  /* for Berkeley DB hash table */

DB *create_bdb(DB *db_ptr);
DB *init_bdb(DB *db_ptr);
DB *remove_db(DB *db_ptr, char *hash_schema);
DB *write_db(DB *db_ptr, char *hash_schema, char *buffer, int size);
int read_db(DB *db_ptr, char *hash_schema, char *buffer);
int get_max_port(DB *db_ptr, int *max_port_in, int *max_port_out);
int close_bdb(DB *db_ptr);

#endif
/* End of database.h */
