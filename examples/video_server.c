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
#include <packedobjectsd/packedobjectsd.h>

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
  /* if (xmlStrcmp(cur->name, (const xmlChar *) "video")) { */
  /*   fprintf(stderr,"document of the wrong type, root node != video"); */
  /*   xmlFreeDoc(req); */
  /*   return -1; */
  /* } */

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

int main(int argc, char *argv [])
{ 
  int ret;
  xmlDocPtr req = NULL;
  packedobjectsdObject *pod_obj = NULL;
  const char *xml_file = "video.xml";
  const char *schema_file = "video.xsd";
 
  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  printf("waiting for search request...\n");
  do { 
  if((req = packedobjectsd_receive(pod_obj)) == NULL) {
    printf("message could not be received\n");
    exit(EXIT_FAILURE);
  }

  /* to ignore messages sent by itself */
  ret = get_frequency(req, "/video/message/search");
  }while(ret == -1); /* Waits until it receives a valid frequency */
 
  printf("search request received with frequency %d\n",ret);
 
  while(1) 
    {     
      xmlDocPtr doc_sent = NULL;
      if((doc_sent = packedobjects_new_doc(xml_file)) == NULL) {
	printf("did not find .xml file");
	exit(EXIT_FAILURE);
      }
  
      if(packedobjectsd_send(pod_obj, doc_sent) == -1){
	printf("message could not be sent\n");
	exit(EXIT_FAILURE);
      }
      printf("new release information is sent\n");
      //packedobjects_dump_doc(doc_sent);
      xmlFreeDoc(doc_sent);
  
      sleep(ret);
    }

  /* free up memory but we should never reach here! */
  xmlFreeDoc(req);
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
