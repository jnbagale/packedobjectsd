
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef ADDRESS_H_
#define ADDRESS_H_

#define MAX_ADDRESS_SIZE 20 /* the maximum size for network address */

typedef struct {
  long pid;
  unsigned int port_in;
  unsigned int port_out;
  char *address;  
} Address;

Address *make_address_object(); 
Address *create_address(Address *addr, char *address, int port_in, int port_out, long pid ); 
void free_address_object(Address *addr); 
int serialize_address(char *buffer, Address *addr); 
int deserialize_address(char *buffer, Address *addr);

#endif
/* End of address.h */
