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
#include <uuid/uuid.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
#define XML_SCHEMA "video.xsd"
static char user_id[36];

/* function prototypes */
int read_response(xmlDocPtr doc_response, xmlChar *xpath);
xmlDocPtr create_search(packedobjectsdObject *pod_obj, char *user_id, char *movie_title, char *max_price);
void *receive_response(void *pod_obj);
void *send_search(void *pod_obj);

/* function definitions */
xmlDocPtr create_search(packedobjectsdObject *pod_obj, char *user_id, char *movie_title, char *max_price)
{
  /* Declare variables */
  xmlDocPtr doc_search = NULL;
  xmlNodePtr video_node = NULL, message_node = NULL, search_node = NULL;
  
  LIBXML_TEST_VERSION;

  ///////////////////// Creating Root and other parent nodes ///////////////////

  doc_search = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  video_node = xmlNewNode(NULL, BAD_CAST "video");
  xmlDocSetRootElement(doc_search, video_node);

  message_node = xmlNewChild(video_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  search_node = xmlNewChild(message_node, NULL, BAD_CAST "search", BAD_CAST NULL);
    
  ///////////////////// Creating child elements inside response node ///////////////////
  
  /* create child elements to hold data */
  xmlNewChild(search_node, NULL, BAD_CAST "sender-id", BAD_CAST user_id);
  xmlNewChild(search_node, NULL, BAD_CAST "movie-title", BAD_CAST movie_title);
  xmlNewChild(search_node, NULL, BAD_CAST "max-price", BAD_CAST max_price);

  // xmlFreeDoc(doc_search);
  return doc_search;
}

int read_response(xmlDocPtr doc_response, xmlChar *xpath)
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
	  
	  if((strcmp(sender_id, user_id) == 0)) {
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

void *receive_response(void *pod_obj)
{
  int ret;
  xmlDocPtr doc_response = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;
 
  ///////////////////// Receiving search response ///////////////////
  while(1)
    {
      if((doc_response = packedobjectsd_receive(pod_object)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
      
      /* ignore if sender-id doesn't match its own id */
      ret = read_response(doc_response, "/video/message/response");
      if(ret == 1) {
      	printf("search response received...\n");
      	xml_dump_doc(doc_response);
      }
      xmlFreeDoc(doc_response);
      usleep(1000);
    }
}

void *send_search(void *pod_obj)
{
  int ret;
  int quit = 0;
  char quit_str[10];
  char movie_title[500];
  char max_price[50];
  xmlDocPtr doc_search = NULL;
  packedobjectsdObject *pod_object =  (packedobjectsdObject *) pod_obj;
  
  ///////////////////// Creating search request ///////////////////
  while(quit != 3) 
    {
      printf("Enter 1 to create new search request\n");
      printf("Enter 2 to send search request\n");
      printf("Enter 3 to quit the program\n");
      quit = atoi(gets(quit_str));

      switch(quit) 
	{
	case 1:
	  printf("Please enter your search details below\n");
	  printf("Please enter the video title\n");
	  gets(movie_title);
	  printf("Please enter the maximum price\n");
	  gets(max_price);

	  if(doc_search != NULL) {
	    xmlFreeDoc(doc_search); // free xml doc pointer if used
	  }
	  doc_search = create_search(pod_object, user_id, movie_title, max_price);
	  //xml_dump_doc(doc_search);
	  break;
	case 2:
	  /* send the search doc to the clients */
	  if(doc_search == NULL) {
	    printf("Please create a search request using option 1 first\n");
	    break;
	  }
	  ///////////////////// Sending search request broadcast ///////////////////
	  if(packedobjectsd_send(pod_object, doc_search) == -1){
	    printf("message could not be sent\n");
	    exit(EXIT_FAILURE);
	  }
	  printf("search request sent to the clients...\n");
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
}

/* main function */
int main(int argc, char *argv [])
{ 
  /* Declare variables */
  uuid_t buffer;
  pthread_t thread_receiver;
  pthread_t thread_searcher;
  packedobjectsdObject *pod_obj = NULL;
  
  ///////////////////// Initialising packedobjectsd ///////////////////

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  ///////////////////// Generating unique id ///////////////////

  uuid_generate_random(buffer);
  uuid_unparse(buffer, user_id);
  printf("searcher's unique user id: %s\n", user_id);


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
