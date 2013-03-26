/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple mobile search program for video data. The searcher will receive video data */
/* from responders at regular interval using packedobjectsd library */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_SCHEMA "video.xsd"

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  xmlDocPtr doc_received = NULL;
  packedobjectsdObject *pod_obj = NULL;
  
  printf(" ///////////////////// VIDEO SEARCHER VERSION-0.1 ////////////////// \n");
    
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, SEARCHER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Receiving video releases ///////////////////
  printf("listening to the video responder ...\n");
  while(1) 
    {       
      /* waiting to receive message */
      if((doc_received = packedobjectsd_receive_response(pod_obj)) == NULL){
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      /* Display the received XML data */
      xml_dump_doc(doc_received);
    
      xmlFreeDoc(doc_received);
      usleep(1000);
    }

  ///////////////////// Freeing ///////////////////

  /* free up memory created by  packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
