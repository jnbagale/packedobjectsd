/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A video library server example. The server will broadcast new action and comedy movie */
/* releases to subscribed clients using packedobjectsd library */

#include <stdio.h>
#include <packedobjectsd/packedobjectsd.h>

int main(int argc, char *argv [])
{ 
  xmlDocPtr doc_sent_action = NULL;
  xmlDocPtr doc_sent_comedy = NULL;
  const char *xml_file_action = "action.xml";
  const char *xml_file_comedy = "comedy.xml";
  const char *schema_file_action = "video.xsd";
  const char *schema_file_comedy = "video.xsd";
  packedobjectsdObject *pod_obj_action = NULL;
  packedobjectsdObject *pod_obj_comedy = NULL;

  /* Initialise packedobjectsd for action movies */
  if((pod_obj_action = init_packedobjectsd(schema_file_action)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  if((doc_sent_action = packedobjects_new_doc(xml_file_action)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }

  /* Initialise packedobjectsd for comedy movies */
  if((pod_obj_comedy = init_packedobjectsd(schema_file_comedy)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  if((doc_sent_comedy = packedobjects_new_doc(xml_file_comedy)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }

  if(packedobjectsd_send(pod_obj_action, doc_sent_action) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }

  printf("new action movie released\n");
  packedobjects_dump_doc(doc_sent_action);

  if(packedobjectsd_send(pod_obj_comedy, doc_sent_comedy) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }

  printf("new comedy movie released\n");
  packedobjects_dump_doc(doc_sent_comedy);

  /* free packedobjectsd */
  free_packedobjectsd(pod_obj_action);
  free_packedobjectsd(pod_obj_comedy);

  return EXIT_SUCCESS;
}
