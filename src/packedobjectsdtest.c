/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A sample ZeroMQ node which can act as both publisher and subscriber */
/* Subscriber connects to broker's outbound socket */
/* Publisher connects to broker's inbound socket */

#include <stdio.h>
#include <string.h>   /* for strcmp0()*/

#include "packedobjectsd/packedobjectsd.h"

int main (int argc, char *argv [])
{
  int ret;
  int encode_type = ENCODED; /* ENCODED or PLAIN */
  int port = DEFAULT_SERVER_PORT ; /* Port number where lookup server is running */
  char *address = DEFAULT_SERVER_ADDRESS; /* IP address where lookup server is running */
  char *pub_schema_path = "../schema/schema.xsd";
  char *sub_schema_path = "../schema/schema.xsd";
  char *message1 = "packedobjectsd test";
 
  /* Initialise objects and variables  */
  pubObject *pub_obj = NULL;
  subObject *sub_obj= NULL;

  if((pub_obj = make_pub_object()) == NULL){
    return -1;
  }  
  if((sub_obj = make_sub_object()) == NULL) {
    return -1;
  }
  
  pub_obj->port = port;
  pub_obj->address = malloc (strlen(address) + 1);
  sprintf(pub_obj->address, "%s",address);
  
  sub_obj->port = port;
  sub_obj->address = malloc (strlen(address) + 1);
  sprintf(sub_obj->address, "%s",address);

  /* Connects to SUB socket */ 
  if((sub_obj = subscribe_to_broker(sub_obj, sub_schema_path)) == NULL) {
    return -1;
  }
   
  /* Connects to PUB socket */
  if((pub_obj = publish_to_broker(pub_obj, pub_schema_path)) == NULL) {
    return -1;
  }
  
  ret = send_data(pub_obj, message1, strlen(message1), encode_type); 
  if(ret != -1) {
    printf("Message sent: %s\n", message1);
    sub_obj = receive_data(sub_obj);
    if(strcmp(message1, sub_obj->message) !=0) {
      printf("Message could not be received!\n");
    }
    else {
      printf("Message received: %s\n", sub_obj->message);
    }
  }
  else {
      printf("Message could not be sent\n");
      return -1;
    }

  return 0;
}

/* End of packedobjectsdtest.c */
