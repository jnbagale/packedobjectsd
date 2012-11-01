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
#include <unistd.h>
#include <packedobjectsd/packedobjectsd.h>

int main(int argc, char *argv [])
{ 
  xmlDocPtr req = NULL;
  const char *request = "search.xml";  /* should be created on the memory? */
  const char *schema_file = "video.xsd";
  packedobjectsdObject *pod_obj = NULL;

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  if((req = packedobjects_new_doc(request)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }

  while(1) {
  /* send search request to the video server */
  if(packedobjectsd_send(pod_obj, req) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("search request sent to the video server\n");
  usleep(1000);
  }
  /* free up memory */
  xmlFreeDoc(req);
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
