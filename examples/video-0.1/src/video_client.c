/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple video library example. The client will broadcast new movie */
/* releases to subscribed searchers using packedobjectsd library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "database.xml"
#define XML_SCHEMA "video.xsd"

static int frequency = 30; /* static variable to be used for controlling sending interval */

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  xmlDocPtr doc_sent = NULL;
  packedobjectsdObject *pod_obj = NULL;
  
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, RESPONDER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Sending ///////////////////

  while(1) 
    {     
      /* initialise new xml document */
      if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
	printf("did not find .xml file");
	exit(EXIT_FAILURE);
      }  

      /* send video xml document to receiver */
      if(packedobjectsd_send_response(pod_obj, doc_sent) == -1){
	printf("message could not be sent\n");
	exit(EXIT_FAILURE);
      }
      printf("video database is sent\n");
      xmlFreeDoc(doc_sent);
      sleep(frequency);
    }


  ///////////////////// Freeing ///////////////////

  /* free up memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
