/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A packedobjectsd test program which implements both publisher and subscriber */
/* Subscriber connects to server and receives messages */
/* Publisher connects to server and sends messages */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   /* for strcmp()*/
#include <unistd.h>  /* for sleep() */
#include <getopt.h>

#include "packedobjectsd.h"

static int verbose_flag;

static void send_file(packedobjectsdObject *pod_obj, const char *file_xml);
static void receive_file(packedobjectsdObject *pod_obj);
static void exit_with_message(char *message);
static void print_usage(void);

static void send_file(packedobjectsdObject *pod_obj, const char *file_xml)
{
  int ret;
  xmlDocPtr doc_sent = NULL;

  if((doc_sent = packedobjects_new_doc(file_xml)) == NULL) {
    exit_with_message("did not find .xml file");
    }

    ret = send_data(pod_obj, doc_sent);
    if(ret == -1) {
      exit_with_message("message could not be sent\n");
    }
    printf("message sent\n");
    //packedobjects_dump_doc(doc_sent);
    xmlFreeDoc(doc_sent);
}

static void receive_file(packedobjectsdObject *pod_obj)
{ 
  xmlDocPtr doc_received = NULL;

  if((doc_received = receive_data(pod_obj)) == NULL) {
   exit_with_message("message could not be received\n");
  }
  printf("message received\n");
  // packedobjects_dump_doc(doc_received);
  xmlFreeDoc(doc_received);
  
}

static void exit_with_message(char *message)
{
  printf("Failed to run: %s\n", message);
  exit(EXIT_FAILURE);
}

static void print_usage(void)

{

  printf("usage: packedobjectsd --schema <file> --xml <file> \n");

  exit(EXIT_SUCCESS);

}

int main (int argc, char *argv [])
{
  packedobjectsdObject *pod_obj = NULL;
  const char *file_xml = NULL;
  const char *file_schema = NULL;
  int loop = 1;
  int c;

while(1) {
    static struct option long_options[] =
      {
        {"verbose", no_argument, &verbose_flag, 1},
        {"help",  no_argument, 0, 'h'},
        {"schema",  required_argument, 0, 's'},
        {"xml",  required_argument, 0, 'x'},
        {"loop",  required_argument, 0, 'l'},        
        {0, 0, 0, 0}
      };

    int option_index = 0;
    c = getopt_long (argc, argv, "hs:x:l:?", long_options, &option_index);
    if (c == -1) break;
    switch (c)
      {
      case 0:
        if (long_options[option_index].flag != 0) break;
        printf ("option %s", long_options[option_index].name);
        if (optarg) printf (" with arg %s", optarg);
        printf ("\n");
        break;
      case 'h':
        print_usage();
        break;  
      case 's':
       file_schema = optarg;
        break;
      case 'x':
        file_xml = optarg;
        break;
      case 'l':
        loop = atoi(optarg);
        break;        
      case '?':
        print_usage();
        break;
      default:
        abort ();
     }
  }

  /* Initialise packedobjectsd */
  if((pod_obj = packedobjectsd_init(file_schema)) == NULL) {
    exit_with_message("failed to initialise libpackedobjectsd\n");
  }
  sleep(1); /* Allow broker to start if it's not already running */

  while(loop) {
    send_file(pod_obj, file_xml);
    receive_file(pod_obj);
    usleep(1000); /* Do nothing for 1 ms */
    loop--;
  }
  // xmlCleanupParser();
  /* free packedobjectsd */
  packedobjectsd_free(pod_obj);

  return EXIT_SUCCESS;
}

/* End of packedobjectsdtest.c */
