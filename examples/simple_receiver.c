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
#include "packedobjectsd.h"

static int query_schema(xmlDocPtr req, xmlChar *xpath);

static int query_schema(xmlDocPtr req, xmlChar *xpath)
{
  xmlXPathContextPtr xpathp = NULL;
  xmlXPathObjectPtr result = NULL;

  /* setup xpath context */
  xpathp = xmlXPathNewContext(req);
  if (xpathp == NULL) {
    printf("Error in xmlXPathNewContext.");
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  if(xmlXPathRegisterNs(xpathp, (const xmlChar *)NSPREFIX, (const xmlChar *)NSURL) != 0) {
    printf("Error: unable to register NS.");
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  /* Evaluate xpath expression */
  result = xmlXPathEvalExpression(xpath, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);

  return 1;
}

int main(int argc, char *argv [])
{ 
  packedobjectsdObject *pod_obj = NULL;
  const char *schema_file = "video.xsd";

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  printf("listening to the video server ...\n");
  while(1) 
    {       
      xmlDocPtr doc_received = NULL;
      if((doc_received = packedobjectsd_receive(pod_obj)) == NULL){
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
      /* to ignore messages sent by the searcher program */
      if((query_schema(doc_received, "/video/message/response")) == 1) {
	printf("video database is received\n");
	packedobjects_dump_doc(doc_received);
      }
      xmlFreeDoc(doc_received);
      usleep(1000);
    }

  /* free up memory but we should never reach here! */

  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
