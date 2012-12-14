/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple program to broadcast video search to connected clients.  */
/* Clients interested in the video then respond to this searcher     */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "search.xml"
#define XML_SCHEMA "video.xsd"

/* function prototypes */
int process_response(xmlDocPtr doc_response, xmlChar *xpath);
void receive_response(packedobjectsdObject *pod_obj);
void broadcast_search(packedobjectsdObject *pod_obj);

/* function definitions */
int process_response(xmlDocPtr doc_response, xmlChar *xpath)
{
  /* Declare variables */
  char *sender_id = NULL;
  xmlXPathContextPtr xpathp = NULL;
  xmlXPathObjectPtr result = NULL;

  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathp = xmlXPathNewContext(doc_response);
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

  ///////////////////// Evaluating XPATH expression ///////////////////

  /* evaluate xpath expression */
  result = xmlXPathEvalExpression(xpath, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  /* check if  xml doc matches "/video/message/response" */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  ///////////////////// Processing XML document ///////////////////

  /* the xml doc matches "/video/message/response" */
  xmlNodePtr cur = xmlDocGetRootElement(doc_response);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"sender-id")))
	{
	  while(cur != NULL) 
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"sender-id")))
		{
		  xmlChar *key;
		  key = xmlNodeListGetString(doc_response, cur->xmlChildrenNode, 1);
		  sender_id = strdup((char *)key);
		  xmlFree(key);	  
		}
	      
	      cur = cur->next; /* traverse to the next XML element */
	    }
	  printf("sender id %s\n", sender_id);

	  if((strcmp(sender_id, "21081203") == 0)) {
	    free(sender_id);
	    return 1;
	  }
	  break; /* exit while loop */
	}
      cur = cur->xmlChildrenNode; /* traverse to next xml node */

    }

  ///////////////////// Freeing ///////////////////

  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);
  
  return -1;
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  int ret;
  packedobjectsdObject *pod_obj = NULL;
  
  ///////////////////// Initialising packedobjectsd ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Sending search broadcast ///////////////////
  
  /* initialising search XML document */
  xmlDocPtr doc_sent = NULL;
  if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
    printf("did not find .xml file");
    exit(EXIT_FAILURE);
  }  

  /* sending search XML document */
  if(packedobjectsd_send(pod_obj, doc_sent) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("search broadcast sent...\n");
  xml_dump_doc(doc_sent);
  /* freeing */
  xmlFreeDoc(doc_sent);
 
  ///////////////////// Receiving search response ///////////////////

  while(1)
    {
      printf("waiting for search response...\n");
      xmlDocPtr doc_response = NULL;
      if((doc_response = packedobjectsd_receive(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
      xml_dump_doc(doc_response);

      /* ignore if sender-id doesn't match its own id */
      ret = process_response(doc_response, "/video/message/response");
      if(ret == 1) {
      	printf("search response received...\n");
      	xml_dump_doc(doc_response);
      }
      xmlFreeDoc(doc_response);
    }

  ///////////////////// Freeing ///////////////////
 
  free_packedobjectsd(pod_obj);
  return EXIT_SUCCESS;
}
