
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <stdio.h>
#include <stdlib.h>  /* for exit()  */
#include <string.h> /* for strlen()*/
#include <zmq.h>   /* ZeroMQ request reply connection */
#include <unistd.h>      /* for fork()  */
#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>  /* for wait() */
#include <errno.h>    /* errno */

#include "config.h"
#include "server.h"
#include "broker.h"
#include "message.h"
#include "database.h"

#define MAX_BROKER_PROCESS 100
static  int current = 0;

serverObject *make_server_object(void)
{
  serverObject *server_obj;

  if ((server_obj = (serverObject *)malloc(sizeof(serverObject))) == NULL) {
    printf("failed to malloc serverObject!");
    exit(EXIT_FAILURE);
  }

  return server_obj;
}

pid_t create_new_broker(serverObject *server_obj)
{
  /* Initialisation of broker object & variables */
  printf("Forking new child process for broker\n");
  pid_t broker_pid = fork();
  if(broker_pid < 0) {
    printf("failed to fork broker child process \n");
    return -1;
  }
  else if(broker_pid == 0) {
    brokerObject *broker_obj;
    broker_obj = make_broker_object();
    broker_obj = init_broker(broker_obj, "127.0.0.1", 5556, 8100);
    printf("Starting a new broker...\n");
    start_broker(broker_obj);    /* Run the broker */
  }

  return broker_pid;
}

void *start_server(void *server_object)
{
  int rc;
  int size;
  int buffer_size;
  int node_type;
  char *rep_endpoint;
  char *node = NULL; 
  char *request = NULL;
  char *buffer = NULL;
  serverObject *server_obj;
  pid_t pid[MAX_BROKER_PROCESS];

  server_obj =  (serverObject *) server_object; /* Casting void * pointer back to serverObject pointer */

  /* Initialise the berkeley database */
  //server_obj->db_ptr = create_bdb(server_obj->db_ptr);
  server_obj->db_ptr = init_bdb(server_obj->db_ptr);

  /* Prepare the context and server socket */
  server_obj->context = zmq_init (1);
  server_obj->responder = zmq_socket (server_obj->context, ZMQ_REP);
  if (server_obj->responder == NULL){
      printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE);
    }

  size = strlen(server_obj->address) + sizeof(int) + 7; /* 7 bytes for 'tcp://' and ':' */
  rep_endpoint = malloc(size + 1); 
  sprintf(rep_endpoint, "tcp://%s:%d", server_obj->address, server_obj->port);

  rc = zmq_bind (server_obj->responder, rep_endpoint);
  if (rc == -1){
      printf("Error occurred during zmq_bind(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE);
    }
 
  while (1) {
  
    /* Wait for next request from client */
    printf ("\nWaiting for request...\n");
    node = receive_message(server_obj->responder, &size);
    if(node != NULL) {
      sscanf(node, "%d", &node_type);
      printf("%s sent a request\n",which_node(node_type));
    }
    else {
      node_type = SUBSCRIBER; /* Assume node type as SUBSCRIBER if node type information is not sent by the node */
    }

    request = receive_message_more(server_obj->responder, &size);
    if(request != NULL) {
      if ((buffer = malloc(MAX_BUFFER_SIZE)) == NULL) {
	printf("Failed to allocate buffer!\n");
      }
    }

    /* check for the schema hash on the database and get back Address structure */
    buffer_size = read_db(server_obj->db_ptr, request, buffer);
    if(buffer_size < 0) {
      if(node_type == PUBLISHER) {
	//remove_db(server_obj->db_ptr, request);
	/* TODO:- Start a new broker and return the new broker details */ 
	server_obj->db_ptr = write_db(server_obj->db_ptr, request);
	buffer_size = read_db(server_obj->db_ptr, request, buffer);
	pid[current] = create_new_broker(server_obj);
	if( pid[current] >= 0) {
	  current++;
	}
      }
      else if(node_type == SUBSCRIBER) {
	printf("No broker details found... \n");
	buffer = NULL;
	buffer_size = 0;
      }
    }
    
    /* Send reply back to client */
    rc = send_message (server_obj->responder, buffer, buffer_size);
    if (rc == -1){
      printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    }
    
    /* int status; */
    /* pid_t pid; */
    /* printf("Waiting for broker child to quit\n"); */
    /* pid = wait(&status); */
    /* printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status); */
    usleep(10000); /* Sleep for 10 milliseconds to allow other threads to run */
  }

  /* We should never reach here unless something goes wrong!  */
  return server_obj;
}

void free_server_object(serverObject *server_obj)
{
  /* Freeing up memory and closing objects */
  if(server_obj != NULL) {
  close_bdb(server_obj->db_ptr);
  zmq_close(server_obj->responder);
  zmq_close(server_obj->requester);
  zmq_term (server_obj->context);
  free(server_obj->address);
  free(server_obj);  
  }
  else {
    printf("The server_obj struct pointer is NULL\n");
   }
}
/* End of server.c */
