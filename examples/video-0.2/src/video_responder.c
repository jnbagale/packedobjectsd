/* Copyright (C) 2009-2012 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A simple mobile search program for video data. The responder will receive search request */
/* from searchers and will respond back video data if match found on database */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_DATA "video.xml"
#define XML_SCHEMA "video.xsd"

/* function prototypes */
int send_response(packedobjectsdObject *pod_obj, char *movie_title, double price);
int prepare_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price);
int process_search(packedobjectsdObject *pod_obj, xmlDocPtr search);

/* function definitions */
int send_response(packedobjectsdObject *pod_obj, char *movie_title, double price)
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
    return -1;
  }
  printf("response sent to the searcher...\n");
  //xml_dump_doc(doc_response);

  xmlFreeDoc(doc_response);
  return 1;
}

int prepare_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price)
{
  /* Declare variables */
  int ret = 1;
  double price = 0.0;
  char *title = NULL;
  xmlDocPtr doc_database = NULL;

  printf("checking video database...\n");

  ///////////////////// Initialising XML document ///////////////////

  if((doc_database = xml_new_doc(XML_DATA)) == NULL) {
    printf("did not find database.xml file");
    return -1;
  }  

  ///////////////////// Processing XML document ///////////////////

  xmlNodePtr cur = xmlDocGetRootElement(doc_database);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"title")))
	{
	  while(cur != NULL)
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"title")))
		{
		  xmlChar *key;
		  key = xmlNodeListGetString(doc_database, cur->xmlChildrenNode, 1);
		  title = strdup((char *)key);
		  xmlFree(key);	  
		}

	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"price"))) {
		xmlChar *key;
		key = xmlNodeListGetString(doc_database, cur->xmlChildrenNode, 1);
		price =	atof((char *)key);
		xmlFree(key);
	      }
	      cur = cur->next;  /* traverse to the next XML element */
	    }
	  // printf("movie title %s price %g\n", title, price);
	  break; /* exit while loop */
	}	     
      cur = cur->xmlChildrenNode;  /* traverse to next xml node */
    }
  
  ///////////////////// Comparing search data with video database ///////////////////

  /* compare the search video title and max price with video title and price from database */
  if((strcmp(movie_title, title) == 0)) {
    if(price <= max_price) {
      printf("the movie exists on the database and matches price limit\n");
      
      ///////////////////// Sending  search response ///////////////////

      /* send response to searcher */
      ret = send_response(pod_obj, movie_title, price);
    }
    else {
      printf("the movie exists on the database but does not match price limit\n");
    }
  }
  else {
    printf("the movie does not exist on the database\n");
  }

  ///////////////////// Freeing ///////////////////

  free(title);
  xmlFreeDoc(doc_database);
  return ret;
}

int process_search(packedobjectsdObject *pod_obj, xmlDocPtr doc_search)
{
  /* Declare variables */
  int i;
  int ret;
  int size;
  double max_price = 0.0;
  char *movie_title = NULL;
  char xpath_exp[1000];
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
  
  sprintf(xpath_exp, "/video/message/search/*"); // xpath expression which should return the movie-title and max-price
    
  result = xmlXPathEvalExpression((const xmlChar *)xpath_exp, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  ///////////////////// Processing search broadcast ///////////////////

  /* the xml doc matches "/video/message/search/asterik(*)" */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    printf("the search request is not in valid format\n");
    return -1;
  }
  else {
    size = result->nodesetval->nodeNr;
    xmlNodePtr cur = result->nodesetval->nodeTab[0];

    for(i = 0; i < size; i++) 
      {
	if(!(xmlStrcmp(cur->name, (const xmlChar *)"movie-title")))
	  {
	    xmlChar *key;
	    key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
	    movie_title = strdup((char *)key);
	    xmlFree(key);	  
	  }

	if(!(xmlStrcmp(cur->name, (const xmlChar *)"max-price")))
	  {
	    xmlChar *key;
	    key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
	    max_price = atof((char *)key);
	    xmlFree(key);	  
	  }
	//printf("cur- name %s \n", cur->name);
	cur = cur->next;
      }
  }

  printf("\n************** search request details **************\n");
  printf("Movie title: %s \n", movie_title);
  printf("Maximum price: %g\n", max_price);
  printf("Searcher's id:- %s\n\n", pod_obj->last_searcher_id);

  ///////////////////// Checking on database ///////////////////

  /* checking if search broadcast matches record on the database */
  ret = prepare_response(pod_obj, movie_title, max_price);
  
  ///////////////////// Freeing ///////////////////

  free(movie_title);
  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);
  
  return ret;
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  int ret;
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_obj = NULL;
  
  printf(" ///////////////////// VIDEO RESPONDER VERSION-0.2 ////////////////// \n");
  ///////////////////// Initialising ///////////////////

  if((pod_obj = init_packedobjectsd(XML_SCHEMA, RESPONDER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Receiving search request ///////////////////

  while(1)
    {
      printf("waiting for search request\n");
      if((doc_search = packedobjectsd_receive_search(pod_obj)) == NULL) {
	printf("message could not be received\n");
	continue;
      }
 
      ///////////////////// Processing search request ///////////////////

      if((ret = process_search(pod_obj, doc_search)) == -1) {
	printf("search request couldn't be processed.../n");	
      }

      xmlFreeDoc(doc_search);
      usleep(1000);
    }

  ///////////////////// Freeing ///////////////////

  /* free memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
