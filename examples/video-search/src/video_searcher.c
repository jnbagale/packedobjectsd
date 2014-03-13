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
#include <pthread.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_SCHEMA "video.xsd"

/* function prototypes */
int read_response(xmlDocPtr doc_response);
void *receive_response(void *pod_obj);
xmlDocPtr create_search(packedobjectsdObject *pod_obj, char *movie_title, char *max_price);
void *send_search(void *pod_obj);
static inline char *get_input(char *buffer, size_t size);

/* function definitions */
static inline char *get_input(char *buffer, size_t size)
{
  if ( fgets(buffer, size, stdin) != NULL)
    {
      strtok(buffer, "\n");
      return buffer;
    }
  return NULL;

}

int read_response(xmlDocPtr doc_response)
{
  /* Declare variables */
  int i;
  int size;
  double movie_price = 0.0;
  char *responder_id = NULL;
  char *movie_title = NULL;
  char xpath_exp[1000];
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

  sprintf(xpath_exp, "/video/message/response/*");

  result = xmlXPathEvalExpression((const xmlChar*)xpath_exp, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  ///////////////////// Processing result ///////////////////

  /* check if xml doc consists of response data" */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    printf("the response data is not in valid format\n");
    return -1;
  }
  else {
    size = result->nodesetval->nodeNr;
    xmlNodePtr cur = result->nodesetval->nodeTab[0];

    for(i = 0; i < size; i++) 
      {
	if(!(xmlStrcmp(cur->name, (const xmlChar *)"responder-id"))) {
	  xmlChar *key;
	  key = xmlNodeListGetString(doc_response, cur->xmlChildrenNode, 1);
	  responder_id = strdup((char *)key);
	  xmlFree(key);
	}

	if(!(xmlStrcmp(cur->name, (const xmlChar *)"movie-title"))) {
	  xmlChar *key;
	  key = xmlNodeListGetString(doc_response, cur->xmlChildrenNode, 1);
	  movie_title = strdup((char *)key);
	  xmlFree(key);
	}

	if(!(xmlStrcmp(cur->name, (const xmlChar *)"price")))
	  {
	    xmlChar *key;
	    key = xmlNodeListGetString(doc_response, cur->xmlChildrenNode, 1);
	    movie_price = atof((char *)key);
	    xmlFree(key);	  
	  }

	cur = cur->next;
      }

    printf("\n********** search response details ***********\n");
    printf("Responder ID: %s \n", responder_id);
    printf("Movie title: %s \n", movie_title);
    printf("Movie price: %g\n", movie_price);
  }
  ///////////////////// Freeing ///////////////////

  xmlXPathFreeObject(result); 
  xmlXPathFreeContext(xpathp);
  
  return 1;
}

void *receive_response(void *pod_obj)
{
  int ret;
  xmlDocPtr doc_response = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;
 
  ///////////////////// Receiving search response ///////////////////
  while(1)
    {
      if((doc_response = packedobjectsd_receive_response(pod_object)) == NULL) {
	printf("message could not be received\n");
	//exit(EXIT_FAILURE);
      }

      printf("\nnew search response received... \n");
      /* process the received response XML */
      if((ret = read_response(doc_response)) != 1) {
      	//xml_dump_doc(doc_response);
     	printf("search response could not be processed \n");
      }

      xmlFreeDoc(doc_response);
      usleep(1000);
    }
}

xmlDocPtr create_search(packedobjectsdObject *pod_obj, char *movie_title, char *max_price)
{
  /* Declare variables */
  xmlDocPtr doc_search = NULL;
  xmlNodePtr video_node = NULL, message_node = NULL, search_node = NULL;
  
  ///////////////////// Creating Root and other parent nodes ///////////////////

  doc_search = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  video_node = xmlNewNode(NULL, BAD_CAST "video");
  xmlDocSetRootElement(doc_search, video_node);

  message_node = xmlNewChild(video_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  search_node = xmlNewChild(message_node, NULL, BAD_CAST "search", BAD_CAST NULL);
    
  ///////////////////// Creating child elements inside response node ///////////////////
  
  /* create child elements to hold data */
  xmlNewChild(search_node, NULL, BAD_CAST "movie-title", BAD_CAST movie_title);
  xmlNewChild(search_node, NULL, BAD_CAST "max-price", BAD_CAST max_price);

  // xmlFreeDoc(doc_search);
  return doc_search;
}

void *send_search(void *pod_obj)
{
  int quit = 0;
  char quit_str[10];
  char movie_title[500];
  char max_price[50];
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;
  
  ///////////////////// Creating search request ///////////////////
  while(quit != 3) 
    {
      printf("Enter 1 to create & send new search request\n");
      printf("Enter 2 to resend current search request\n");
      printf("Enter 3 to quit the program\n");
      quit = atoi(get_input(quit_str, sizeof quit_str));

      switch(quit) 
	{
	case 1:
	  printf("Please enter your search details below\n");
	  printf("Please enter the video title\n");
	  if(get_input(movie_title, sizeof movie_title) == NULL) {
	    printf("Title input unsuccessful\n");
	  }
	  printf("Please enter the maximum price\n");
	  if(get_input(max_price, sizeof max_price) == NULL) {
	    printf("Price input unsuccessful\n");
	  }	    

	  if(doc_search != NULL) {
	    xmlFreeDoc(doc_search); // free xml doc pointer if used
	  }
	  doc_search = create_search(pod_object, movie_title, max_price);
	  //xml_dump_doc(doc_search);
	  
	  /* send the search doc to the clients */
	  if(doc_search == NULL) {
	    printf("Search request could not be processed\n");
	    break;
	  }

	  ///////////////////// Sending search request broadcast ///////////////////
	  if(packedobjectsd_send_search(pod_object, doc_search) == -1){
	    printf("message could not be sent\n");
	    exit(EXIT_FAILURE);
	  }
	  printf("search request sent to the responders...\n");
	  //xml_dump_doc(doc_search);
	  break;

	case 2:
	  	  /* send the search doc to the clients */
	  if(doc_search == NULL) {
	    printf("Create a new search request using option 1\n");
	    break;
	  }

	  ///////////////////// Sending search request broadcast ///////////////////
	  if(packedobjectsd_send_search(pod_object, doc_search) == -1){
	    printf("message could not be sent\n");
	    exit(EXIT_FAILURE);
	  }
	  printf("search request sent to the responders...\n");
	  //xml_dump_doc(doc_search);
	  break;

	case 3:
	  printf("This program is now quitting...\n");
	  exit(EXIT_SUCCESS);
	  break;
	default:
	  printf("Unknown option\n");
	  break;
	}
    }
  return pod_object;
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  pthread_t thread_receiver;
  pthread_t thread_searcher;
  packedobjectsdObject *pod_obj = NULL;

  printf("///////////////////// VIDEO SEARCHER /////////////////// \n");

  ///////////////////// Initialising packedobjectsd ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA, SEARCHER, NO_COMPRESSION)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  printf("Connected to POD with schema hash: %s\n", pod_obj->schema_hash);
  printf("Sending search requestes to %s\n", pod_obj->publisher_endpoint);
  printf("Receiving search responses from %s\n", pod_obj->subscriber_endpoint);
  printf("POD Node ID %lu\n\n", pod_obj->unique_id);
  
  
  ///////////////////// Initialising threads ///////////////////

  /* initialise thread to execute send_search() function */
  if (pthread_create( &thread_searcher, NULL, send_search, (void *) pod_obj)) {
    fprintf(stderr, "Error creating searcher thread \n");
    exit(EXIT_FAILURE);
  }

  /* initialise thread to execute receive_response() function */
  if (pthread_create( &thread_receiver, NULL, receive_response, (void *) pod_obj)) {
    fprintf(stderr, "Error creating receiver thread \n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Sending search request ///////////////////

  /* Join the thread to send search request */
  if(pthread_join( thread_searcher, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }
   
  ///////////////////// Receiving search response ///////////////////

  /* Join the thread to receive search response */
  if(pthread_join( thread_receiver, NULL)) {
    fprintf(stderr, "Error joining searcher thread \n");
    exit(EXIT_FAILURE);
  }
  
  ///////////////////// Freeing ///////////////////
 
  free_packedobjectsd(pod_obj);
  return EXIT_SUCCESS;
}
