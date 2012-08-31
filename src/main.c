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
#include <string.h>   /* for strcmp()*/
#include <unistd.h>  /* for sleep() */

#include "packedobjectsd.h"

int main (int argc, char *argv [])
{
  int ret;
  xmlDocPtr doc_sent = NULL;
  xmlDocPtr doc_received = NULL;
  char *path_xml = "../schema/personnel.xml";
  char *path_schema = "../schema/personnel.xsd";
 
  /* Initialise objects and variables  */
  packedobjectsdObject *pod_obj = NULL;
  if((pod_obj = packedobjectsd_init(path_schema)) == NULL) {
    return -1;
  }
  sleep(1); /* Allow broker to start if it's not already running */
 
  doc_sent = packedobjects_new_doc((const char *) path_xml);
  if(doc_sent != NULL) {
    ret = send_data(pod_obj, doc_sent); 
    if(ret != -1) {
      printf("Message sent\n");
      doc_received = receive_data(pod_obj);
      if(doc_received == NULL) {
	printf("Message could not be received!\n");
      }
      else {
	printf("Message received\n");
      }
    }
    else {
      printf("Message could not be sent\n");
    }
  }

  packedobjects_dump_doc(doc_received);
  xmlFreeDoc(doc_received);
  xmlFreeDoc(doc_sent);
  packedobjectsd_free(pod_obj);

  return 0;
}

/* End of packedobjectsdtest.c */
