/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple mobile search program for video data. The responder will send video data */
/* to searchers at regular interval using packedobjectsd library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "video.xml"
#define XML_SCHEMA "video.xsd"

static int frequency = 30; /* static variable in seconds used as sending frequency */

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  xmlDocPtr doc_sent = NULL;
  packedobjectsdObject *pod_obj = NULL;
    
  printf(" ///////////////////// VIDEO RESPONDER VERSION-0.1 ////////////////// \n");
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, PUBLISHER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Sending ///////////////////
  printf("sending video data to the video searcher ...\n");
  while(1) 
    {     
      sleep(frequency);
      
      /* initialise new xml document */
      if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
	printf("did not find .xml file");
	exit(EXIT_FAILURE);
      }  

      /* send video xml document to receiver */
      if(packedobjectsd_send(pod_obj, doc_sent) == -1){
	printf("message could not be sent\n");
	exit(EXIT_FAILURE);
      }
      printf("video database is sent\n");
      xmlFreeDoc(doc_sent);
    }


  ///////////////////// Freeing ///////////////////

  /* free up memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
