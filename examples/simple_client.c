/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple video library client example. The client will receive new video */
/* releases for 'action' movies from server using packedobjectsd library */

#include <stdio.h>
#include <packedobjectsd/packedobjectsd.h>

int main(int argc, char *argv [])
{ 
  xmlDocPtr doc_received = NULL;
  const char *schema_file = "video.xsd";
  packedobjectsdObject *pod_obj = NULL;

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  if((doc_received = packedobjectsd_receive(pod_obj)) == NULL) {
    printf("message could not be received\n");
    exit(EXIT_FAILURE);
  }
  printf("new action movie released\n");
  packedobjects_dump_doc(doc_received);

  /* free up memory */
  xmlFreeDoc(doc_received);
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
