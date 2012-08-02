
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <zmq.h> /* ZeroMQ request reply connection */
#include <db.h>  /* Berkeley db hash table */

#include "config.h"
#include "lookup.h"

serverObject *make_server_object(void)
{
  serverObject *server_obj;

  if ((server_obj = (serverObject *)malloc(sizeof(serverObject))) == NULL) {
    printf("failed to malloc serverObject!");
    exit(EXIT_FAILURE);
  }

  return server_obj;
}

serverObject *init_bdb(serverObject *server_obj)
{

  /* DB structure handle */
  u_int32_t flags;   
  int ret;          

  /* Initialize the structure */
  ret = db_create(&server_obj->db_ptr, NULL, 0);
  if (ret != 0) {
    printf("Error creating database!\n");
    exit(EXIT_FAILURE);
  }

  /* Database open flags */
  flags = 0; /* Database must be created in advance */

  /* open the database */
  ret = server_obj->db_ptr->open(server_obj->db_ptr, NULL, DATABASE, NULL, DB_HASH, flags, 0);

  if (ret != 0) {
    printf("Error opening database!\n");
    exit(EXIT_FAILURE);
  }
  /* else { */
  /*   printf("Database is opened and ready for use\n"); */
  /* } */

  return server_obj;
}

serverObject *write_db(serverObject *server_obj, char *hash_schema)
{
  int ret;
  int size;
  DBT key, data;
  char buffer[MAX_BUFFER];

  /* Start a new broker and return the new broker details */
  Address *addr;
  char *address = "127.0.0.1";
  addr = make_address_object();

  /* if ((addr->address = malloc(MAX_ADDRESS)) == NULL) { */
  /*   printf("Failed to allocate address!\n"); */
  /* } */

  addr = create_address(addr, address , 5556, 8100);

  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));
  
  key.data = hash_schema ;
  key.size = strlen(hash_schema);

  size = serialize_address(buffer, addr); /* add checking for error on serialization */
  data.data = buffer; 
  data.size = size; 

  /* Inserting data to the database */
  ret = server_obj->db_ptr->put(server_obj->db_ptr, NULL, &key, &data, DB_NOOVERWRITE);
  if (ret == DB_KEYEXIST) {
    server_obj->db_ptr->err(server_obj->db_ptr, ret, "Put failed because key %s already exists", hash_schema);
  }
  else {
    printf("The key:- %s is inserted to database successfully\n", hash_schema);
  }
  
  server_obj = close_bdb(server_obj);
  server_obj = init_bdb(server_obj);
  
  return server_obj;
}

int read_db(serverObject *server_obj, char *hash_schema, char *buffer)
{
  /* Accessing data from the database */
  int ret;
  DBT key, data;

  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));

  key.data = hash_schema;
  key.size = strlen(hash_schema);

  data.data = buffer;
  data.ulen = MAX_BUFFER; 
  data.flags = DB_DBT_USERMEM;

  /* Retrieving data from the database */
  ret =  server_obj->db_ptr->get(server_obj->db_ptr, NULL, &key, &data, 0);

  if (ret == DB_NOTFOUND) { 
    server_obj->db_ptr->err(server_obj->db_ptr, ret, "The key:- %s: doesn't exist in database\n", hash_schema);
  }

  return ret;
}

void walkDB(serverObject *server_obj)
{
  DBC *cursorp;
  DBT key, data;
  int ret;
 
  /* Initialize cursor */
  server_obj->db_ptr->cursor(server_obj->db_ptr, NULL, &cursorp, 0);
 
  /* Initialize our DBTs. */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));
 
  /* Iterate over the database, retrieving each record in turn. */
  /* use DB_NEXT for interating forward
   * and DB_PREV for backward */
    while (!(ret=cursorp->c_get(cursorp, &key, &data, DB_NEXT))) 
      {
	printf("%s - %s\n", key.data, data.data);
      }
    if (ret != DB_NOTFOUND) {
      fprintf(stderr,"Nothing found in the database.\n");
      exit(1);
    }
 
    /* Close cursor before exit */
    if (cursorp != NULL)
      cursorp->c_close(cursorp);
}


serverObject *remove_db(serverObject *server_obj, char *hash_schema)
{
  int ret;
  DBT key;
  
  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  key.data = hash_schema;
  key.size = strlen(hash_schema);

  ret = server_obj->db_ptr->del(server_obj->db_ptr, NULL, &key, 0);  
  if(ret != 0) {
    if (ret == DB_NOTFOUND) {
      server_obj->db_ptr->err(server_obj->db_ptr, ret, "The key:- %s: could not be removed because it doesn't exist in database\n", hash_schema);
    }
  }
  else {
    printf("The key:- %s is removed from the database successfully\n", hash_schema);
    server_obj = close_bdb(server_obj);
    server_obj = init_bdb(server_obj);
  }

  return server_obj;

}

serverObject *close_bdb(serverObject *server_obj)
{
  /* If the database is not NULL, close it. */
  if (server_obj->db_ptr != NULL) {
    server_obj->db_ptr->close(server_obj->db_ptr, 0); 
  }
  return server_obj;
}

void *start_server(void *server_object)
{
  int rc;
  int size;
  char *request;
  char *rep_endpoint;
  char *buffer;
  serverObject *server_obj;
  server_obj =  (serverObject *) server_object; /* Casting void * pointer back to serverObject pointer */

  /* Initialise the berkeley database */
  server_obj = init_bdb(server_obj);
  walkDB(server_obj);
  
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
    request = receive_message(server_obj->responder);
    //remove_db(server_obj, "schef826596c9a09633c6ca6a0a51660");
  
    if ((  buffer = malloc(MAX_BUFFER)) == NULL) {
      printf("Failed to allocate buffer!\n");
    }
  
    /* check for the schema hash on the berkeley database and get back Address structure */
    rc = read_db(server_obj, request, buffer);

    if(rc != 0) {
      server_obj = write_db(server_obj, request);
      rc = read_db(server_obj, request, buffer);
    }

    Address *addr;
    addr = make_address_object();
    size = deserialize_address(buffer, addr);

    /* Send reply back to client */
    rc = send_message (server_obj->responder, buffer, size);
    if (rc == -1){
      printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    }

    sleep(1);
  }

  /* We should never reach here unless something goes wrong!  */
  return server_obj;
}

void free_server_object(serverObject *server_obj)
{
  /* Freeing up memory and closing objects */
  if(server_obj != NULL) {
  close_bdb(server_obj);
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
/* End of lookup.c */
