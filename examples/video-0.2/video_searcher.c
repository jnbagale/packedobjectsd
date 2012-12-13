/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple program to broadcast video search to connected clients.  */
/* Clients interested in the video then respond to this searcher     */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
static const char *search_file = "search.xml";
static  const char *schema_file = "video.xsd";

/* function prototypes */
void receive_response(packedobjectsdObject *pod_obj);
void broadcast_search(packedobjectsdObject *pod_obj);

/* function definitions */
void receive_response(packedobjectsdObject *pod_obj)
{
  while(1)
    {
      printf("waiting for search response...\n");
      xmlDocPtr doc_response = NULL;
      if((doc_response = packedobjectsd_receive(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      printf("search response received...\n");
      xml_dump_doc(doc_response);
      xmlFreeDoc(doc_response);
    }
}

void broadcast_search(packedobjectsdObject *pod_obj)
{
  xmlDocPtr doc_sent = NULL;
  printf("sending  search broadcast...\n");

  if((doc_sent = xml_new_doc(search_file)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }  

  if(packedobjectsd_send(pod_obj, doc_sent) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("search broadcast sent...\n");
  xml_dump_doc(doc_sent);

  xmlFreeDoc(doc_sent);
}

/* main function */
int main(int argc, char *argv [])
{ 
  packedobjectsdObject *pod_obj = NULL;
  
  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  /* call function to send search broadcast */
  broadcast_search(pod_obj);

  /* call function to receive search response */
  receive_response(pod_obj);

  free_packedobjectsd(pod_obj);
  return EXIT_SUCCESS;
}
