/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple program to receive video search broadcast  */
/* Response will be sent if search terms are satisfied */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "video.xml"
#define XML_SCHEMA "video.xsd"
static char client_id[36];

/* function prototypes */
void send_response(packedobjectsdObject *pod_obj, char *movie_title, double price);
int prepare_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price);
int process_search(packedobjectsdObject *pod_obj, xmlDocPtr search, char *xpathExpr);

/* function definitions */
void send_response(packedobjectsdObject *pod_obj, char *movie_title, double price)
{
  /* Declare variables */
  char price_string[50];
  xmlDocPtr doc_response = NULL;
  xmlNodePtr video_node = NULL, message_node = NULL, response_node = NULL;
  
  LIBXML_TEST_VERSION;

  ///////////////////// Creating Root and other parent nodes ///////////////////

  doc_response = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  video_node = xmlNewNode(NULL, BAD_CAST "video");
  xmlDocSetRootElement(doc_response, video_node);

  message_node = xmlNewChild(video_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  response_node = xmlNewChild(message_node, NULL, BAD_CAST "response", BAD_CAST NULL);
    
  ///////////////////// Creating child elements inside response node ///////////////////
  
  /* create child elements to hold data */
  sprintf(price_string,"%g", price);
  xmlNewChild(response_node, NULL, BAD_CAST "movie-title", BAD_CAST movie_title);
  xmlNewChild(response_node, NULL, BAD_CAST "price", BAD_CAST price_string);

  ///////////////////// Sending response to the searcher ///////////////////

  /* send the response doc to the searcher */
  if(packedobjectsd_send_response(pod_obj, doc_response) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("response sent to the searcher...\n");
  //xml_dump_doc(doc_response);

  xmlFreeDoc(doc_response);
}

int prepare_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price)
{
  /* Declare variables */
  int i;
  int size;
  double price = 0.0;
  char *title = NULL;
  char xpath_exp[1000];
  xmlDocPtr doc_database = NULL;

  printf("received a search request...\nchecking in video database...\n");
  ///////////////////// Initialising XML document ///////////////////

  if((doc_database = xml_new_doc(XML_DATA)) == NULL) {
    printf("did not find database.xml file");
    exit(EXIT_FAILURE);
  }
  
  xmlXPathContextPtr xpathp = NULL;
  xmlXPathObjectPtr result = NULL;

  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathp = xmlXPathNewContext(doc_database);
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
  
  sprintf(xpath_exp, "/video/message/database/movie[title='%s']/price", movie_title);
    
  /* Evaluate xpath expression */
  result = xmlXPathEvalExpression((const xmlChar *)xpath_exp, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  ///////////////////// Processing result ///////////////////

  /* check if xml doc consists of data with title matching value from movie_title variable" */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    printf("the movie does not exist on the database\n");
    return -1;
  }
  else {
    size = result->nodesetval->nodeNr;
    xmlNodePtr cur = result->nodesetval->nodeTab[0];

    for(i = 0; i < size; i++) 
      {
	if(!(xmlStrcmp(cur->name, (const xmlChar *)"price")))
	  {
	    xmlChar *key;
	    key = xmlNodeListGetString(doc_database, cur->xmlChildrenNode, 1);
	    price = atof((char *)key);
	    xmlFree(key);	  
	  }

	cur = cur->next;
      }

    ///////////////////// Comparing search data with video database ///////////////////

    /* compare max price from search with price from database */
 
    if(price <= max_price) {
      printf("the movie exists on the database and matches price limit\n");
      
      ///////////////////// Sending  search response ///////////////////

      /* send response to searcher */
      send_response(pod_obj, movie_title, price);
    }
    else {
      printf("the movie exists on the database but does not match price limit\n");
    }
  }
 
  ///////////////////// Freeing ///////////////////

  free(title);
  xmlFreeDoc(doc_database);
  return 1;
}

int process_search(packedobjectsdObject *pod_obj, xmlDocPtr doc_search, char *xpathExpr)
{
  /* Declare variables */
  double max_price = 0.0;
  char *movie_title = NULL;
  xmlXPathContextPtr xpathCtxPtr = NULL;
  xmlXPathObjectPtr xpathObjPtr = NULL;

  ///////////////////// Initialising XPATH ///////////////////

  /* setup xpath context */
  xpathCtxPtr = xmlXPathNewContext(doc_search);
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

  /* Evaluate xpath expression */
  xpathObjPtr = xmlXPathEvalExpression((const xmlChar *)xpathExpr, xpathCtxPtr);
  if (xpathObjPtr == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(xpathObjPtr); 
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  /* check if xml doc matches "/video/message/search" */
  if(xmlXPathNodeSetIsEmpty(xpathObjPtr->nodesetval)) {
    xmlXPathFreeObject(xpathObjPtr); 
    xmlXPathFreeContext(xpathCtxPtr);
    return -1;
  }

  ///////////////////// Processing search broadcast ///////////////////

  /* the xml doc matches "/video/message/search" */
  xmlNodePtr cur = xmlDocGetRootElement(doc_search);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"movie-title")))
	{
	  while(cur != NULL)
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"movie-title"))) {
		xmlChar *key;
		key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
		movie_title = strdup((char *)key);
		xmlFree(key);
	      }

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"max-price"))) {
		xmlChar *key;
		key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
		max_price = atof((char *)key);
		xmlFree(key);
	      }
	     
	      cur = cur->next;   /* traverse to the next XML element */
	    }
	  printf("\n            ***** search request details ******\n");
	  printf("              movie title: %s \n", movie_title);
	  printf("              max price: %g\n\n", max_price);

	  ///////////////////// Checking on database ///////////////////

	  /* checking if search broadcast matches record on the database */
	  prepare_response(pod_obj, movie_title, max_price);  
	  break; /* exit the while loop */
	}
      
      cur = cur->xmlChildrenNode;  /* traverse to next XML node */
    }

  ///////////////////// Freeing ///////////////////

  free(movie_title);
  xmlXPathFreeObject(xpathObjPtr); 
  xmlXPathFreeContext(xpathCtxPtr);
  
  return 1;
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_obj = NULL;

  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, RESPONDER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Receiving search broadcast ///////////////////

  while(1)
    {
      /* waiting for search broadcast */
      printf("waiting for search broadcast\n");
      if((doc_search = packedobjectsd_receive_search(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
 
      usleep(100000); /* Allow searcher program some time to prepare receiving */
      // xml_dump_doc(doc_search);

      ///////////////////// Processing search broadcast ///////////////////

      /* process search broadcast to retrieve search details */
      process_search(pod_obj, doc_search, "/video/message/search");
      xmlFreeDoc(doc_search);
    }

  ///////////////////// Freeing ///////////////////

  /* free memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
