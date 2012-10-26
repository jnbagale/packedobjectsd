/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A video library client example. The client will receive new video */
/* releases for 'action' & 'comedy' movies from server using packedobjectsd library */

#include <stdio.h>
#include <packedobjectsd/packedobjectsd.h>

int main(int argc, char *argv [])
{
  xmlDocPtr doc_received_action = NULL;
  xmlDocPtr doc_received_comedy = NULL;
  const char *schema_file_action = "action.xsd";
  const char *schema_file_comedy = "comedy.xsd";
  packedobjectsdObject *pod_obj_action = NULL;
  packedobjectsdObject *pod_obj_comedy = NULL;

  /* Initialise packedobjectsd for action movies */
  if((pod_obj_action = init_packedobjectsd(schema_file_action)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  /* Initialise packedobjectsd for comedy movies */
  if((pod_obj_comedy = init_packedobjectsd(schema_file_comedy)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }
  /* Add thread to enable the program to listen for both action and comedy movies at the same time */
  if((doc_received_action = packedobjectsd_receive(pod_obj_action)) == NULL) {
    printf("message could not be received\n");
    exit(EXIT_FAILURE);
  }
  printf("new action movie released\n");
  packedobjects_dump_doc(doc_received_action);
  
  if((doc_received_comedy = packedobjectsd_receive(pod_obj_comedy)) == NULL) {
    printf("message could not be received\n");
    exit(EXIT_FAILURE);
  }
  printf("new comedy movie released\n");
  packedobjects_dump_doc(doc_received_comedy);

  /* free up memory */
  xmlFreeDoc(doc_received_action);
  xmlFreeDoc(doc_received_comedy);
  free_packedobjectsd(pod_obj_action);
  free_packedobjectsd(pod_obj_comedy);

  return EXIT_SUCCESS;
}
