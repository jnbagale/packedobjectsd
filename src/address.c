
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <stdio.h>
#include <stdlib.h>  /* for free() */
#include <string.h> /* for  memcpy() & strlen() */

#include "address.h"

Address *make_address_object(void) 
{
  Address *addr;

  if ((addr = (Address *) malloc(sizeof(Address))) == NULL) {
    printf("Failed to allocate Address structure!\n");
    return NULL;
  }

  return addr;
}

Address *create_address(Address *addr, char *address, int port_in, int port_out, long pid ) 
{
  addr->pid = pid;
  addr->port_in = port_in;
  addr->port_out = port_out;
  addr->address = malloc(strlen(address) + 1);
  strncpy(addr->address, address, strlen(address));

  return addr;
}

void free_address_object(Address *addr) 
{
  free(addr->address);
  free(addr);
}

int serialize_address(char *buffer, Address *addr) /* Add host to network order code for port numbers */
{
  size_t offset = 0;

  memcpy(buffer, &addr->pid, sizeof(addr->pid));
  offset = sizeof(addr->pid);
  memcpy(buffer + offset, &addr->port_in, sizeof(addr->port_in));
  offset = offset + sizeof(addr->port_in);
  memcpy(buffer + offset, &addr->port_out, sizeof(addr->port_out));
  offset = offset + sizeof(addr->port_out);
  memcpy(buffer + offset, addr->address, strlen(addr->address) + 1);
  offset = offset + strlen(addr->address) + 1;

  return offset;
}

int deserialize_address(char *buffer, Address *addr)  /* Add network to host order code for port numbers */
{
  size_t offset = 0;
   
  if ((addr->address = malloc(MAX_ADDRESS_SIZE)) == NULL) {
    printf("Failed to allocate address!\n");
    return -1;
  }

  memcpy(&addr->pid, buffer, sizeof(addr->pid));
  offset = sizeof(addr->pid);
  memcpy(&addr->port_in, buffer + offset, sizeof(addr->port_in));
  offset = offset + sizeof(addr->port_in);
  memcpy(&addr->port_out, buffer + offset, sizeof(addr->port_out));
  offset = offset + sizeof(addr->port_out);
  memcpy(addr->address, buffer + offset, strlen(buffer + offset) + 1);
  offset = offset + strlen(buffer + offset) + 1;

  return offset;
}

/* End of address.c */
