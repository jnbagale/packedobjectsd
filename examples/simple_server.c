/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple video library server example. The server will broadcast new action movie */
/* releases to subscribed clients using packedobjectsd library */

#include <stdio.h>
#include <packedobjectsd/packedobjectsd.h>

int main(int argc, char *argv [])
{ 
  xmlDocPtr doc_sent = NULL;
  const char *xml_file = "action.xml";
  const char *schema_file = "action.xsd";
  packedobjectsdObject *pod_obj = NULL;

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  if((doc_sent = packedobjects_new_doc(xml_file)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }
  
  if(packedobjectsd_send(pod_obj, doc_sent) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }

  printf("new action movie released\n");
  packedobjects_dump_doc(doc_sent);

  /* free packedobjectsd */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
