
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
#include <db.h>

#include "config.h"
#include "lookup.h"

#define DATABASE "packedobjectsd.db"
#define ADDRESS_SIZE 99

serverObject *make_server_object()
{
  serverObject *server_obj;

  if ((server_obj = (serverObject *)malloc(sizeof(serverObject))) == NULL) {
    //printf("failed to malloc serverObject!");
    exit(EXIT_FAILURE);
  }

  return server_obj;
}

DB *init_bdb(DB *db_ptr)
{

  /* DB structure handle */
  u_int32_t flags;   /* database open flags */
  int ret;           /* function return value */

  /* Initialize the structure. This
   * database is not opened in an environment, 
   * so the environment pointer is NULL. */
  ret = db_create(&db_ptr, NULL, 0);
  if (ret != 0) {
    printf("Error creating database!\n");
    /* Error handling goes here */
    exit(EXIT_FAILURE);
  }

  /* Database open flags */
  flags = 0; //DB_CREATE;    /* If the database does not exist, create it.*/

  /* open the database */
  ret = db_ptr->open(db_ptr, NULL, DATABASE, NULL, DB_HASH, flags, 0);

  if (ret != 0) {
    printf("Error opening database!\n");
    /* Error handling goes here */
    exit(EXIT_FAILURE);
  }
  else {
    printf("Database is opened and ready for use\n");
  }

  return db_ptr;
}

DB *write_db(DB *db_ptr)
{
  int ret;
  DBT key, data;
  
  char *schema_hash = "schema-hash"; /* key */
  char *address = "127.0.0.1"; /* data */
  
  /* Initialize the DBTs */
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));
  
  key.data = schema_hash ;
  key.size = sizeof(schema_hash);
  
  data.data = address;
  data.size = strlen(address) + 1;

  /* Inserting data to the database */
  ret = db_ptr->put(db_ptr, NULL, &key, &data, DB_NOOVERWRITE);
  if (ret == DB_KEYEXIST) {
    db_ptr->err(db_ptr, ret, "Put failed because key %s already exists", schema_hash);
  }
 else
   printf("The key:- %s and data:- %s \nis inserted to database successfully\n", schema_hash, address);
  return db_ptr;
}

void read_db(DB *db_ptr)
{
  /* Accessing data from the database */

  int ret;
  DBT key1, data1;
  char address1[ADDRESS_SIZE + 1]; /* data */
  char *schema_hash1 = "schema-hash"; /* key */
   
 /* Initialize the DBTs */
  memset(&key1, 0, sizeof(DBT));
  memset(&data1, 0, sizeof(DBT));

  key1.data = schema_hash1;
  key1.size = sizeof(schema_hash1);

  data1.data = address1;
  data1.ulen = ADDRESS_SIZE + 1;
  data1.flags = DB_DBT_USERMEM;

  /* Retrieving data from the database */
  ret =  db_ptr->get(db_ptr, NULL, &key1, &data1, 0);
  if (ret == DB_NOTFOUND) {
    db_ptr->err(db_ptr, ret, "The key:- %s: doesn't exist in database\n", schema_hash1);
  }
  else
    printf("The address of the broker for the given schema: %s is:\n %s\n", schema_hash1, address1);
}

void start_server(void)
{
  // dummy function 
  //printf("I do nothing at the moment. I am just a dummy function\n");
}

int store_address(char *schema_hash, char *address)
{
  //dummy function to retrieve and store store schema and address of broker
  return 1;
}

char *lookup_address(char *schema_hash)
{
  //dummy function to listen to look up request from nodes and return address of broker
  return "127.0.0.1";
}

void close_bdb(DB *db_ptr)
{
/* If the database is not NULL, close it. */
  if (db_ptr != NULL) {
    db_ptr->close(db_ptr, 0); 
  }
}
