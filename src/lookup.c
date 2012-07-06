
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

#include "config.h"

void start_server(void)
{
  // dummy function 
  g_print("I do nothing at the moment. I am just a dummy function\n");
}

gint store_address(gchar *schema_hash, gchar *address)
{
  //dummy function to retrieve and store store schema and address of broker
  return 1;
}

gchar *lookup_address(gchar *schema_hash)
{
  //dummy function to listen to look up request from nodes and return address of broker
  return "127.0.0.1";
}
