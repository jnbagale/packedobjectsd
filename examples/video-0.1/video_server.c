/* Copyright (C) 2009-2011 The Clashing Rocks Team */

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

static int frequency = 5;
static const char *xml_file = "video.xml";
static  const char *schema_file = "video.xsd";
static int get_frequency(xmlDocPtr req, xmlChar *xpath);

static int get_frequency(xmlDocPtr req, xmlChar *xpath)
{
  int frequency = -1; 
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

  xmlNodePtr cur = xmlDocGetRootElement(req);
 
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"frequency")))
	{
	  xmlChar *key;
	  key = xmlNodeListGetString(req, cur->xmlChildrenNode, 1);
	  //printf("Frequency: %s\n", key);
	  frequency = atoi(key);
	  xmlFree(key);	  
	}
      cur = cur->xmlChildrenNode;
    }

  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);
  
  return frequency;
}

void *process_searcher(void *pod_obj)
{
  int ret;
  xmlDocPtr req = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;

  while(1)
    {
      if((req = packedobjectsd_receive(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }

      /* to ignore messages sent by itself */
      ret = get_frequency(req, "/video/message/search");
      if(ret != -1) {
	printf("search request received with frequency %d\n",ret);
	frequency = ret;
      }
    }
  xmlFreeDoc(req);
}

void *process_receiver( void *pod_obj)
{
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;

  while(1) 
    {     
      xmlDocPtr doc_sent = NULL;
      if((doc_sent = xml_new_doc(xml_file)) == NULL) {
	printf("did not find .xml file");
	exit(EXIT_FAILURE);
      }  

      if(packedobjectsd_send(pod_obj, doc_sent) == -1){
	printf("message could not be sent\n");
	exit(EXIT_FAILURE);
      }
      // printf("new release information is sent\n");
      xmlFreeDoc(doc_sent);
      sleep(frequency);
    }
}

int main(int argc, char *argv [])
{ 
  pthread_t thread_receiver;
  pthread_t thread_searcher;
  packedobjectsdObject *pod_obj = NULL;
  
  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
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

  /* Join the thread to start the server */
  if(pthread_join( thread_searcher, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }
 
  /* Join the thread to start the server */
  if(pthread_join( thread_receiver, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }
 
  /* free up memory but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
