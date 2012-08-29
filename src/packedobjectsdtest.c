/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A test ZeroMQ node which can act as both publisher and subscriber */
/* Subscriber connects to broker's outbound socket */
/* Publisher connects to broker's inbound socket */

#include <stdio.h>
#include <string.h>   /* for strcmp()*/
#include <unistd.h>  /* for sleep() */

#include "packedobjectsd.h"

int main (int argc, char *argv [])
{
  int ret;
  int size;
  int node_type = 2; /* Subscriber 0; Publisher 1; Both 2 */
  int encode_type = 1; /* Plain 0; Encoded 1 */
  int server_port = DEFAULT_SERVER_PORT ; /* Port number where lookup server is running */
  char *server_address = DEFAULT_SERVER_ADDRESS; /* IP address where lookup server is running */
  char *path_schema = "../schema/schema.xsd";
  char *message = "packedobjectsd test message";
 
  /* Initialise objects and variables  */
  packedobjectsdObject *pod_obj = NULL;

  if((pod_obj = packedobjectsd_init(node_type, path_schema, server_address, server_port)) == NULL) {
    return -1;
  }
  sleep(1); /* Allow broker to start if it's not already running */
 
  size = strlen(message);
  ret = send_data(pod_obj, message, size, encode_type); 
  
  if(ret != -1) {
    printf("Message sent: %s\n", message);
    pod_obj = receive_data(pod_obj);
    if(pod_obj->data_received == NULL) {
      printf("Message could not be received!\n");
    }
    else {
      printf("Message received: %s\n", pod_obj->data_received);
    }
  }
  else {
    printf("Message could not be sent\n");
  }

  packedobjectsd_free(pod_obj);

  return 0;
}

/* End of packedobjectsdtest.c */
