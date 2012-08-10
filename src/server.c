
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
#include <zmq.h>   /* ZeroMQ functions   */
#include <unistd.h>      /* for fork()  */
#include <sys/types.h>  /* pid_t       */
#include <sys/wait.h>  /* for wait()  */
#include <errno.h>    /* errors      */

#include "config.h"
#include "server.h"
#include "broker.h"
#include "message.h"
#include "database.h"

serverObject *make_server_object(void)
{
  serverObject *server_obj;

  if ((server_obj = (serverObject *)malloc(sizeof(serverObject))) == NULL) {
    printf("failed to malloc serverObject!");
    exit(EXIT_FAILURE);
  }

  return server_obj;
}

pid_t fork_new_broker(char *address, int port_in, int port_out)
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
    broker_obj = init_broker(broker_obj, address, port_in, port_out);
    start_broker(broker_obj);    /* Run the broker */
  }

  return broker_pid;
}

serverObject *create_new_broker(serverObject *server_obj, char *request, char *buffer, int *buffer_size) {
 
  pid_t broker_pid;
  Address *addr = make_address_object();
  	    
  broker_pid = fork_new_broker(server_obj->address, server_obj->max_port_in, server_obj->max_port_out);

  if(broker_pid >= 0) {
    addr = create_address(addr, server_obj->address, server_obj->max_port_in, server_obj->max_port_out, (long) broker_pid);
    *buffer_size = serialize_address(buffer, addr); /* add checking for error on serialization */
    server_obj->db_ptr = write_db(server_obj->db_ptr, request, buffer, *buffer_size);
    server_obj->count++;
    server_obj->max_port_in++;
    server_obj->max_port_out++;
   }

  free_address_object(addr); /* Free up Address structure */
  return server_obj;
}


void *start_server(void *server_object)
{
  int rc;
  int size;
  int node_type; 
  int buffer_size = 0;
  char *rep_endpoint;
  char *node = NULL; 
  char *buffer = NULL;
  char *request = NULL;
  serverObject *server_obj;

  server_obj =  (serverObject *) server_object; /* Casting void * pointer back to serverObject pointer */

  /* Initialise the berkeley database */
  //server_obj->db_ptr = create_bdb(server_obj->db_ptr);
  server_obj->db_ptr = init_bdb(server_obj->db_ptr);
  get_max_port(server_obj->db_ptr, &server_obj->max_port_in, &server_obj->max_port_out);

  /* Prepare the context and server socket */
  server_obj->context = zmq_init (1);
  server_obj->responder = zmq_socket (server_obj->context, ZMQ_REP);
  if (server_obj->responder == NULL){
      printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE); /* Handle the error!!! */
    }

  size = strlen(server_obj->address) + sizeof(int) + 7; /* 7 bytes for 'tcp://' and ':' */
  rep_endpoint = malloc(size + 1); 
  sprintf(rep_endpoint, "tcp://%s:%d", server_obj->address, server_obj->port);

  rc = zmq_bind (server_obj->responder, rep_endpoint);
  if (rc == -1){
      printf("Error occurred during zmq_bind(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE); /* Handle the error!!! */
    }
 
  while (1) {
  
    /* Wait for next request from node */
    printf ("\nWaiting for request...\n");
    node = receive_message(server_obj->responder, &size);
    if(node != NULL) {
      sscanf(node, "%d", &node_type);
      printf("Received broker detail request from a %s\n",which_node(node_type));
    }
    else {
      printf("Could not determine requesting node type! Assuming it as a SUBSCRIBER\n");
      node_type = SUBSCRIBER; /* Assume node type as SUBSCRIBER if node type information is not sent by the node */
    }

    request = receive_message_more(server_obj->responder, &size);
    if(request == NULL) {
      printf("Received request is null! Sending back null reply... \n");
    }
    else {
      if ((buffer = malloc(MAX_BUFFER_SIZE)) == NULL) {
	printf("Failed to allocate buffer!\n");
      }
      else {
	/* check for the schema hash on the database and get back Address structure */
	//if(node_type == PUBLISHER) server_obj->db_ptr = remove_db(server_obj->db_ptr, request);

	buffer_size = read_db(server_obj->db_ptr, request, buffer);	
	if(buffer_size < 0) {
	  if(node_type == PUBLISHER) {
	    server_obj = create_new_broker(server_obj, request, buffer, &buffer_size);
	  }
	  else {
	    printf("No broker exists for the given schema! Sending back null reply... \n");
	    buffer = NULL;
	    buffer_size = 0;
	  }
	} else {
	  Address *addr = make_address_object();
	  buffer_size = deserialize_address(buffer, addr);

	  if(node_type == PUBLISHER) {

	    if (kill(addr->pid, 0) == 0) {
	      //printf("process is already running %ld In %d Out %d\n", addr->pid, addr->port_in, addr->port_out);

	    } else if (errno == ESRCH) {
	      printf("Broker process [%ld] is not running! Starting new broker process...\n", addr->pid);
	      server_obj->db_ptr = remove_db(server_obj->db_ptr, request);
	      server_obj->max_port_in = addr->port_in;
	      server_obj->max_port_out = addr->port_out;
	      server_obj = create_new_broker(server_obj, request, buffer, &buffer_size);
	      get_max_port(server_obj->db_ptr, &server_obj->max_port_in, &server_obj->max_port_out);

	      /* no such process with the given pid is running */
	    } else {
	      /* some other error... use perror("...") or strerror(errno) to report */
	    }  
	  }
	  free_address_object(addr); /* Free up Address structure */
	}
      }
    }
   
    /* Send reply back to client */
    rc = send_message (server_obj->responder, buffer, buffer_size);
    if (rc == -1){
      printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    }
    else {
      printf("Sent back broker detail for the given schema...\n");
    }

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
