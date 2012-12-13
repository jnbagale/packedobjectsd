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
#include <pthread.h>  
#include <string.h>
#include <packedobjectsd/packedobjectsd.h>

/* global variables */
static const char *xml_file = "database.xml";
static  const char *schema_file = "video.xsd";

/* function prototypes */
void send_response(packedobjectsdObject *pod_obj, char *client_id, char *movie_title, double price, char *sender_id);
void process_response(packedobjectsdObject *pod_obj, char *sender_id, char *movie_title, double max_price);
void process_search(packedobjectsdObject *pod_obj);
int retrieve_details(packedobjectsdObject *pod_obj, xmlDocPtr search, xmlChar *xpath);


/* function definitions */
void send_response(packedobjectsdObject *pod_obj, char *client_id, char *movie_title, double price, char *sender_id)
{
  /* create xml doc to send to the searcher */
  char price_string[50];
  sprintf(price_string,"%g", price);
 
  xmlDocPtr doc_response = NULL;
  xmlNodePtr video_node = NULL, message_node = NULL, response_node = NULL;
  
  LIBXML_TEST_VERSION;

  doc_response = xmlNewDoc(BAD_CAST "1.0");

  /* create pod node as root node */
  video_node = xmlNewNode(NULL, BAD_CAST "video");
  xmlDocSetRootElement(doc_response, video_node);

  message_node = xmlNewChild(video_node, NULL, BAD_CAST "message", BAD_CAST NULL);
  response_node = xmlNewChild(message_node, NULL, BAD_CAST "response", BAD_CAST NULL);
    
  /* create nodes to hold data */
  xmlNewChild(response_node, NULL, BAD_CAST "sender-id", BAD_CAST sender_id);
  xmlNewChild(response_node, NULL, BAD_CAST "client-id", BAD_CAST client_id);
  xmlNewChild(response_node, NULL, BAD_CAST "movie-title", BAD_CAST movie_title);
  xmlNewChild(response_node, NULL, BAD_CAST "price", BAD_CAST price_string);

  printf("created xml doc to send to the searcher\n");  
  //xml_dump_doc(doc_response);

  /* send the response doc to the searcher as broadcast */
  printf("sending  search response broadcast...\n");

  if(packedobjectsd_send(pod_obj, doc_response) == -1){
    printf("message could not be sent\n");
    exit(EXIT_FAILURE);
  }
  printf("search response broadcast sent...\n");

  xmlFreeDoc(doc_response);

}

void process_response(packedobjectsdObject *pod_obj, char *sender_id, char *movie_title, double max_price)
{
  double price;
  char *client_id = NULL;
  char *title = NULL;
  xmlDocPtr doc_database = NULL;

  printf("checking video database...\n");

  if((doc_database = xml_new_doc(xml_file)) == NULL) {
    printf("did not find database.xml file");
    exit(EXIT_FAILURE);
  }  

  xmlNodePtr cur = xmlDocGetRootElement(doc_database);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"client-id")))
       	{
	  xmlChar *key;
	  key = xmlNodeListGetString(doc_database, cur->xmlChildrenNode, 1);
	  client_id = strdup(key);
	  xmlFree(key);	
	  
	  cur = cur->next; /* move to next element movie */
	}

      if(!(xmlStrcmp(cur->name, (const xmlChar *)"title")))
	{
	  while(cur != NULL) /* traverse to the next XML element */
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
	      cur = cur->next;
	    }
	  // printf("client id %s movie title %s price %g\n",client_id, title, price);
	  break;
	}
	
      /* traverse to next xml node */
      cur = cur->xmlChildrenNode;
    }

  /* compare the search video title and max price with video title and price from database */
  if( (strcmp(movie_title, title) == 0)) {
    if(price <= max_price) {
      printf("the movie exists on the database and matches price limit\n");
      // send response to searcher
      send_response(pod_obj, client_id, movie_title, price, sender_id);
    }
    else {
      printf("the movie exists on the database but does not match price limit\n");
    }
  }
  else {
    printf("the movie does not exist on the database\n");
  }

  free(client_id);
  free(title);
  xmlFreeDoc(doc_database);
}

int retrieve_details(packedobjectsdObject *pod_obj, xmlDocPtr doc_search, xmlChar *xpath)
{
  double max_price;
  char *sender_id = NULL;
  char *movie_title = NULL;
  xmlXPathContextPtr xpathp = NULL;
  xmlXPathObjectPtr result = NULL;

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

  /* Evaluate xpath expression */
  result = xmlXPathEvalExpression(xpath, xpathp);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression.");
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }
  /* xml doc does not match video/message/search */
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result); 
    xmlXPathFreeContext(xpathp);
    return -1;
  }

  /* the xml doc matched video/message/search  */
  xmlNodePtr cur = xmlDocGetRootElement(doc_search);
  while(cur != NULL)
    {
      if(!(xmlStrcmp(cur->name, (const xmlChar *)"sender-id")))
	{
	  while(cur != NULL) /* traverse to the next XML element */
	    {
	      if(!(xmlStrcmp(cur->name, (const xmlChar *)"sender-id")))
		{
		  xmlChar *key;
		  key = xmlNodeListGetString(doc_search, cur->xmlChildrenNode, 1);
		  sender_id = strdup((char *)key);
		  xmlFree(key);	  
		}
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

	      cur = cur->next;
	    }
	  printf("\n            ***** search request details ******\n");
	  printf("              sender id: %s \n", sender_id);
	  printf("              movie title: %s \n", movie_title);
	  printf("              max price: %g\n\n", max_price);

	  /* checking if search request matches data on the database */
	  process_response(pod_obj, sender_id, movie_title, max_price);

	  free(sender_id);
	  free(movie_title);
	  break;
	}
	
      /* traverse to next xml node */
      cur = cur->xmlChildrenNode;
    }

xmlXPathFreeObject(result); 
xmlXPathFreeContext(xpathp);
  
return 1;
}

void process_search(packedobjectsdObject *pod_obj)
{
  int ret;
 
  while(1)
    {
      printf("waiting for search request...\n");
      xmlDocPtr doc_search = NULL;
      if((doc_search = packedobjectsd_receive(pod_obj)) == NULL) {
	printf("message could not be received\n");
	exit(EXIT_FAILURE);
      }
      printf("search request received and started processing...\n");
      // xml_dump_doc(doc_search);
      ret = retrieve_details(pod_obj, doc_search, "/video/message/search");
      if(ret != -1) {
	printf("search request processed successfully\n");
      }
      xmlFreeDoc(doc_search);
    }
}

int main(int argc, char *argv [])
{ 
  packedobjectsdObject *pod_obj = NULL;
  
  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file)) == NULL) {
    printf("failed to initialise libpackedobjectsd\n");
    exit(EXIT_FAILURE);
  }

  /* call function to process search request */
  process_search(pod_obj);

  /* free up memory but we should never reach here! */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}
