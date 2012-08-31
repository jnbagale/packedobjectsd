
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <zmq.h>  /* for ZeroMQ functions */

int send_message(void *socket, char *message, int message_length); 
int send_message_more(void *socket, char *message, int message_length); 
char *receive_message(void *socket, int *size); 
char *receive_message_more(void *socket, int *size);

#endif
/* End of message.h */
