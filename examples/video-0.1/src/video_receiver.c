/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple video library client example. The client will receive new video */
/* releases for movies from server using packedobjectsd library */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_SCHEMA "video.xsd"

static int query_schema(xmlDocPtr doc_received, char *xpathExpr);

static int query_schema(xmlDocPtr doc_received, char *xpathExpr)
{
  /* Declare variables */
  xmlXPathContextPtr xpathCtxPtr = NULL;
  xmlXPathObjectPtr xpathObjPtr = NULL;

  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathCtxPtr = xmlXPathNewContext(doc_received);
  if (xpathCtxPtr == NULL) {
    printf("Error in xmlXPathNewContext.");
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  if(xmlXPathRegisterNs(xpathCtxPtr, (const xmlChar *)NSPREFIX, (const xmlChar *)NSURL) != 0) {
    printf("Error: unable to register NS.");
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  ///////////////////// Evaluating XPATH expression ///////////////////

  /* evaluate xpath expression */
  xpathObjPtr = xmlXPathEvalExpression((const xmlChar*)xpathExpr, xpathCtxPtr);
  if (xpathObjPtr == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(xpathObjPtr); 
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  /*check if xml doc matches "/video/message/response" */
  if(xmlXPathNodeSetIsEmpty(xpathObjPtr->nodesetval)) {
    xmlXPathFreeObject(xpathObjPtr); 
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  ///////////////////// Freeing ///////////////////

  xmlXPathFreeObject(xpathObjPtr); 
  xmlXPathFreeContext(xpathCtxPtr);

  return 1;
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  int count = 0;
  xmlDocPtr doc_received = NULL;
  packedobjectsdObject *pod_obj = NULL;
    
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Receiving video releases ///////////////////

  printf("listening to the video server ...\n");
  while(1) 
    {       
      /* waiting to receive message */
      if((doc_received = packedobjectsd_receive(pod_obj)) == NULL){
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      /* to ignore messages sent by the searcher program */
      if((query_schema(doc_received, "/video/message/response")) == 1) {
	count++;
	printf("new video release information is received # %d\n", count);
	xml_dump_doc(doc_received);
      }
      xmlFreeDoc(doc_received);
      usleep(1000);
    }

  ///////////////////// Freeing ///////////////////

  /* free up memory created by  packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
