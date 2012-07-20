
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
#include <db.h>
 
typedef struct {
  
  DB *db_ptr;


}serverObject;

DB *init_bdb();
DB *write_db(DB *db_ptr);
void read_db(DB *db_ptr);
void close_bdb(DB *db_ptr);
void start_server(void);
gint store_address(gchar *schema_hash, gchar *address);
gchar *lookup_address(gchar *schema_hash);
