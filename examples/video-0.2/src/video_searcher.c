/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple mobile search program for video data. The searcher will send search request */
/* to responders and will receive back video data from responders */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "search.xml"
#define XML_SCHEMA "video.xsd"

/* function prototypes */
int read_response(xmlDocPtr doc_response, char *xpathExpr);

/* function definitions */
int read_response(xmlDocPtr doc_response, char *xpathExpr)
{
  /* Declare variables */
  xmlXPathContextPtr xpathCtxPtr = NULL;
  xmlXPathObjectPtr xpathObjPtr = NULL;

  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathCtxPtr = xmlXPathNewContext(doc_response);
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

  /* check if  xml doc matches "/video/message/response" */
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
  int ret;
  xmlDocPtr doc_sent = NULL;
  xmlDocPtr doc_response = NULL;
  packedobjectsdObject *pod_obj = NULL;
  
  printf(" ///////////////////// VIDEO SEARCHER VERSION-0.2 ////////////////// \n");
  ///////////////////// Initialising packedobjectsd ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, SEARCHER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }
  // wait for init setup to finish
  sleep(1);

  ///////////////////// Sending search broadcast ///////////////////
  
  /* initialising search XML document */
  if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }  

  /* sending search XML document */
  if(packedobjectsd_send_search(pod_obj, doc_sent) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("search broadcast sent...\n");
  //xml_dump_doc(doc_sent);
  /* freeing */
  xmlFreeDoc(doc_sent);
 
  ///////////////////// Receiving search response ///////////////////
  
  while(1)
    {
      printf("waiting for search response...\n");
      if((doc_response = packedobjectsd_receive_response(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      /* check if the data is a response msg */
      ret = read_response(doc_response, "/video/message/response");
      if(ret == 1) {
      	printf("search response received...\n");
      	xml_dump_doc(doc_response);
      }
      xmlFreeDoc(doc_response);
      usleep(1000);
    }

  ///////////////////// Freeing ///////////////////
 
  free_packedobjectsd(pod_obj);
  return EXIT_SUCCESS;
}
