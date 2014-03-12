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

/* function prototypes */
void send_response(packedobjectsdObject *pod_obj, char *movie_title, double price);
int create_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price);
int process_search(packedobjectsdObject *pod_obj, xmlDocPtr search);

/* function definitions */
void send_response(packedobjectsdObject *pod_obj, char *movie_title, double price)
{
  /* Declare variables */
  char price_string[50];
  xmlDocPtr doc_response = NULL;
  xmlNodePtr video_node = NULL, message_node = NULL, response_node = NULL;
  
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

int create_response(packedobjectsdObject *pod_obj, char *movie_title, double max_price)
{
  /* Declare variables */
  int i;
  int size;
  double price = 0.0;
  char *title = NULL;
  char xpath_exp[1000];
  xmlDocPtr doc_database = NULL;

  printf("checking in video database...\n");
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
  printf("Searcher's id:- %lu\n\n", pod_obj->last_searcher);

  ///////////////////// Checking on database ///////////////////

  /* checking if search broadcast matches record on the database */
  ret = create_response(pod_obj, movie_title, max_price);
  
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
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_obj = NULL;

  printf("///////////////////// VIDEO RESPONDER  /////////////////// \n");
  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, RESPONDER)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  printf("Connected to POD with schema hash: %s\n", pod_obj->schema_hash);
  printf("Sending search responses to %s\n", pod_obj->subscriber_endpoint);
  printf("Receiving search requests from %s\n", pod_obj->publisher_endpoint);
  printf("POD Node ID %lu\n\n", pod_obj->unique_id);

  ///////////////////// Receiving search broadcast ///////////////////

  while(1)
    {
      /* waiting for search broadcast */
      printf("waiting for new search broadcast\n");
      if((doc_search = packedobjectsd_receive_search(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
      
      printf("\nnew search broadcast received... \n");
      
      ///////////////////// Processing search broadcast ///////////////////

      /* process search broadcast to retrieve search details */
      process_search(pod_obj, doc_search);
      xmlFreeDoc(doc_search);
    }

  ///////////////////// Freeing ///////////////////

  /* free memory created by packedobjectsd but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
