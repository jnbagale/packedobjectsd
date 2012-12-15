/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple video library server example. The server will broadcast new movie */
/* releases to subscribed clients using packedobjectsd library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>  
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "video.xml"
#define XML_SCHEMA "video.xsd"

static int frequency = 10;

/* function prototype */
static int get_frequency(xmlDocPtr doc_search, xmlChar *xpath);
void *process_receiver(void *pod_obj);

/* function definitions */
static int get_frequency(xmlDocPtr doc_search, xmlChar *xpath)
{
  /* Declare variables */
  int ret; 
  xmlXPathContextPtr xpathp = NULL;
  xmlXPathObjectPtr result = NULL;
  
  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathp = xmlXPathNewContext(doc_search);
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

  /* check if the xml doc matches "/video/message/search" */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  ///////////////////// Processing XML document ///////////////////

  xmlNodePtr cur = xmlDocGetRootElement(doc_search);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"frequency")))
	{
	  xmlChar *key;
	  key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
	  //printf("Frequency: %s\n", key);
	  ret = atoi((char *)key);
	  xmlFree(key);	  
	}
      cur = cur->xmlChildrenNode;
    }

  ///////////////////// Freeing ///////////////////

  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);
  
  return ret;
}

void *process_searcher(void *pod_obj)
{
  int ret;
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;

  ///////////////////// Receiving frequency ///////////////////

  while(1)
    {
      /* waiting for new frequency from searcher */
      if((doc_search = packedobjectsd_receive(pod_object)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      /* to ignore messages sent by itself */
      ret = get_frequency(doc_search, "/video/message/search");
      if(ret != -1) {
	printf("search request received with frequency %d\n",ret);
	frequency = ret;
      }
    }
  xmlFreeDoc(doc_search);
}

void *process_receiver(void *pod_obj)
{
  xmlDocPtr doc_sent = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;
  
  ///////////////////// Sending ///////////////////

  while(1) 
    {     
      /* initialise new xml document */
      if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
	printf("did not find .xml file");
	exit(EXIT_FAILURE);
      }  

      /* send video xml document to receiver */
      if(packedobjectsd_send(pod_object, doc_sent) == -1){
	printf("message could not be sent\n");
	exit(EXIT_FAILURE);
      }
      // printf("new release information is sent\n");
      xmlFreeDoc(doc_sent);
      sleep(frequency);
    }
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  pthread_t thread_receiver;
  pthread_t thread_searcher;
  packedobjectsdObject *pod_obj = NULL;
  
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  printf("waiting for search request...\n");
  /* initialise thread to execute start_searcher() function */
  if (pthread_create( &thread_searcher, NULL, process_searcher,(void *) pod_obj)) {
    fprintf(stderr, "Error creating searcher thread \n");
    exit(EXIT_FAILURE);
  }

  /* initialise thread to execute start_searcher() function */
  if (pthread_create( &thread_receiver, NULL, process_receiver,(void *) pod_obj)) {
    fprintf(stderr, "Error creating receiver thread \n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Listening to searcher ///////////////////

  /* Join the thread to listen to searcher  */
  if(pthread_join( thread_searcher, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }
   
  ///////////////////// Sending video releases ///////////////////

  /* Join the thread to send video to receiver */
  if(pthread_join( thread_receiver, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Freeing ///////////////////

  /* free up memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_FAILURE;
}
