/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A test ZeroMQ node which can act as both publisher and subscriber */
/* Subscriber connects to broker's outbound socket */
/* Publisher connects to broker's inbound socket */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   /* for strcmp()*/
#include <unistd.h>  /* for sleep() */

#include "packedobjectsd.h"

static void send_file(packedobjectsdObject *pod_obj, char *file_xml);
static void receive_file(packedobjectsdObject *pod_obj);
static void exit_with_message(char *message);

static void send_file(packedobjectsdObject *pod_obj, char *file_xml)
{
  int ret;
  xmlDocPtr doc_sent = NULL;

  if((doc_sent = packedobjects_new_doc((const char *)file_xml)) == NULL) {
    exit_with_message("did not find .xml file");
    }

    ret = send_data(pod_obj, doc_sent);
    if(ret == -1) {
      exit_with_message("message could not be sent\n");
    }
    printf("message sent\n");
    //packedobjects_dump_doc(doc_sent);
    xmlFreeDoc(doc_sent);
}

static void receive_file(packedobjectsdObject *pod_obj)
{ 
  xmlDocPtr doc_received = NULL;

  if((doc_received = receive_data(pod_obj)) == NULL) {
   exit_with_message("message could not be received\n");
  }
  printf("message received\n");
  // packedobjects_dump_doc(doc_received);
  xmlFreeDoc(doc_received);
  
}

static void exit_with_message(char *message)
{
  printf("Failed to run: %s\n", message);
  exit(EXIT_FAILURE);
}

int main (int argc, char *argv [])
{
  packedobjectsdObject *pod_obj = NULL;
  char *file_xml = "../schema/personnel.xml";
  char *file_schema = "../schema/personnel.xsd";
 
  /* Initialise packedobjectsd */
  if((pod_obj = packedobjectsd_init(file_schema)) == NULL) {
    exit_with_message("failed to initialise libpackedobjectsd\n");
  }
  sleep(1); /* Allow broker to start if it's not already running */

  while(1) {
    send_file(pod_obj, file_xml);
    receive_file(pod_obj);
    usleep(1000); /* Do nothing for 1 ms */
  }
  xmlCleanupParser();
  /* free packedobjectsd */
  packedobjectsd_free(pod_obj);

  return EXIT_SUCCESS;
}

/* End of packedobjectsdtest.c */
