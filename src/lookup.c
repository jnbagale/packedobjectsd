
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
#include <stdlib.h> 
#include <string.h>
#include <zmq.h> /* ZeroMQ request reply connection */
#include <db.h>  /* Berkeley db hash table */

#include "config.h"
#include "lookup.h"

#define DATABASE "packedobjectsd.db"
#define ADDRESS_SIZE 99

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
  u_int32_t flags;   /* database open flags */
  int ret;           /* function return value */

  /* Initialize the structure */
  ret = db_create(&server_obj->db_ptr, NULL, 0);
  if (ret != 0) {
    printf("Error creating database!\n");
    /* Error handling goes here */
    exit(EXIT_FAILURE);
  }

  /* Database open flags */
  flags = 0; //DB_CREATE;   /* If the database does not exist, create it.*/

  /* open the database */
  ret = server_obj->db_ptr->open(server_obj->db_ptr, NULL, DATABASE, NULL, DB_HASH, flags, 0);

  if (ret != 0) {
    printf("Error opening database!\n");
    /* Error handling goes here */
    exit(EXIT_FAILURE);
  }
  else {
    printf("Database is opened and ready for use\n");
  }

  return server_obj;
}

serverObject *write_db(serverObject *server_obj, char *schema_hash)
{
  int ret;
  DBT key, data;
  
  char *address = "127.0.0.1"; /* data */
  
  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));
  
  key.data = schema_hash ;
  key.size = strlen(schema_hash);
  
  data.data = address;
  data.size = strlen(address) + 1;

  /* Inserting data to the database */
  ret = server_obj->db_ptr->put(server_obj->db_ptr, NULL, &key, &data, DB_NOOVERWRITE);
  if (ret == DB_KEYEXIST) {
    server_obj->db_ptr->err(server_obj->db_ptr, ret, "Put failed because key %s already exists", schema_hash);
  }
  else {
    printf("The key:- %s and data:- %s \nis inserted to database successfully\n", schema_hash, address);
  }

  server_obj = close_bdb(server_obj);
  server_obj = init_bdb(server_obj);

  return server_obj;
}

char *read_db(serverObject *server_obj, char *schema_hash)
{
  /* Accessing data from the database */

  int ret;
  DBT key, data;
  char address[ADDRESS_SIZE + 1]; /* data */
  char *broker_address = NULL;
    
 /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));

  key.data = schema_hash;
  key.size = strlen(schema_hash);

  data.data = address;
  data.ulen = ADDRESS_SIZE + 1;
  data.flags = DB_DBT_USERMEM;

  /* Retrieving data from the database */
  ret =  server_obj->db_ptr->get(server_obj->db_ptr, NULL, &key, &data, 0);
  if (ret == DB_NOTFOUND) {
    server_obj->db_ptr->err(server_obj->db_ptr, ret, "The key:- %s: doesn't exist in database\n", schema_hash);

    /* Start a new broker and return the new broker details */
    server_obj = write_db(server_obj, schema_hash);
    return read_db(server_obj, schema_hash);
  }
  else {  
    printf("The address of the broker for the given schema: %s is:\n %s\n", schema_hash, address);
    broker_address = malloc(strlen(address) +1 );
    sprintf(broker_address, "%s",address); 
    return broker_address;

  }
 
}

serverObject *remove_db(serverObject *server_obj, char *schema_hash)
{
  int ret;
  DBT key;
  
  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  key.data = schema_hash;
  key.size = strlen(schema_hash);

  ret = server_obj->db_ptr->del(server_obj->db_ptr, NULL, &key, 0);  
  if(ret != 0) {
    if (ret == DB_NOTFOUND) {
      server_obj->db_ptr->err(server_obj->db_ptr, ret, "The key:- %s: doesn't exist in database\n", schema_hash);
    }
  }
  else {
    printf("The key:- %s is removed from the database successfully",schema_hash);
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

void *start_server(void *server_obj)
{
  int rc;
  int size;
  char *rep_endpoint;
  serverObject *server_object;
  server_object =  (serverObject *) server_obj; /* Casting void * pointer back to serverObject pointer */

  /* Initialise the berkeley database */
  server_object = init_bdb(server_object);

  server_object->context = zmq_init (1);

  /* Socket to talk to clients */
  server_object->responder = zmq_socket (server_object->context, ZMQ_REP);
  if (server_object->responder == NULL){
      printf("Error occurred during zmq_socket(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE);
    }

  size = strlen(server_object->address);
  rep_endpoint = malloc(size + sizeof (int) + 7 + 1); /* 7 bytes for 'tcp://' and ':' */
  sprintf(rep_endpoint, "tcp://%s:%d", server_object->address, 5555);

  rc = zmq_bind (server_object->responder, rep_endpoint);
  if (rc == -1){
      printf("Error occurred during zmq_bind(): %s\n", zmq_strerror (errno));
      exit(EXIT_FAILURE);
    }

  while (1) {
  
    /* Wait for next request from client */
    printf ("\nWaiting for request...\n");
    char *data, *address;
    zmq_msg_t request;
    zmq_msg_init (&request);

    rc = zmq_recv (server_object->responder, &request, 0);
    if (rc == -1){
      printf("Error occurred during zmq_recv(): %s\n", zmq_strerror (errno));
    }

    size = zmq_msg_size (&request);
    data = malloc(size + 1);
    memcpy ( data, zmq_msg_data (&request), size);
    data[size] = 0;
 
    /* check for the schema hash on the berkeley database */
    address = read_db(server_obj, data);

    zmq_msg_close (&request);
    free(data);

    /* Send reply back to client */
    size = strlen(address);
    zmq_msg_t reply;
    zmq_msg_init_size (&reply, size);
    memcpy (zmq_msg_data (&reply), address, size);

    rc = zmq_send (server_object->responder, &reply, 0);
    if (rc == -1){
      printf("Error occurred during zmq_send(): %s\n", zmq_strerror (errno));
    }
    zmq_msg_close (&reply);
    sleep(1);
  }

  /* We should never reach here unless something goes wrong!  */
  return server_object;
}

void free_server_object(serverObject *server_obj)
{
  /* Freeing up memory and closing objects */
  printf("Freeing up memory and quitting the program now...\n");
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
