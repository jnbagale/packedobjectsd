
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <db.h>  /* for Berkeley DB hash table */
#include <stdio.h>
#include <stdlib.h>  /* for exit() */ 
#include <string.h> /* for  memset() & strlen() */
#include <zmq.h>   /* for ZeroMQ functions */

#include "message.h"

/* Function prototype declarations */
DB *create_bdb(DB *db_ptr);
DB *init_bdb(DB *db_ptr);
DB *write_db(DB *db_ptr, char *hash_schema, char *buffer, int size);
int read_db(DB *db_ptr, char *hash_schema, char *buffer);
int get_max_port(DB *db_ptr, int *max_port_in, int *max_port_out);
DB *remove_db(DB *db_ptr, char *hash_schema);
int close_bdb(DB *db_ptr);

DB *create_bdb(DB *db_ptr)
{
  int ret;  
  u_int32_t flags;    /* DB structure handle */
         
 
  ret = db_create(&db_ptr, NULL, 0);  /* Initialize the structure */
  if (ret != 0) {
    printf("Error creating database!\n");
    return NULL;
  }

  flags = DB_CREATE; /* Database will be created if it doesn't already exist */
  ret = db_ptr->open(db_ptr, NULL, DATABASE, NULL, DB_HASH, flags, 0);   /* open the database */
  if (ret != 0) {
    printf("Error opening database!\n");
    return NULL;
  }
  else {
    printf("Database is created successfully and is ready for use\n");
  }

  return db_ptr;
}

DB *init_bdb(DB *db_ptr)
{
  int ret;
  u_int32_t flags; /* DB structure handle */
         
  ret = db_create(&db_ptr, NULL, 0);   /* Initialize the structure */
  if (ret != 0) {
    printf("Error creating database!\n");
    return NULL;
  }

  flags = 0; /* Database must be created in advance */
  ret = db_ptr->open(db_ptr, NULL, DATABASE, NULL, DB_HASH, flags, 0);  /* open the database */
  if (ret != 0) {
    printf("Error opening database!\n");
    exit(EXIT_FAILURE); /* Handle error nicely */
  }
  /* else { */
  /*   printf("Database is opened and ready for use\n"); */
  /* } */

  return db_ptr;
}

DB *write_db(DB *db_ptr, char *hash_schema, char *buffer, int size)
{
  int ret;
  DBT key, data;

  memset(&key, 0, sizeof(DBT)); /* Initialize the DBTs */
  memset(&data, 0, sizeof(DBT));
  
  key.data = hash_schema;
  key.size = strlen(hash_schema);
 
  data.data = buffer; 
  data.size = size; 

  ret = db_ptr->put(db_ptr, NULL, &key, &data, DB_NOOVERWRITE);  /* Inserting data to the database */
  if (ret == DB_KEYEXIST) {
    db_ptr->err(db_ptr, ret, "Put failed because key %s already exists", hash_schema);
  }
  else {
    printf("The key:- %s is inserted to database successfully\n", hash_schema);
  }
  
  close_bdb(db_ptr);
  db_ptr = init_bdb(db_ptr);
  return db_ptr;
}

int read_db(DB *db_ptr, char *hash_schema, char *buffer)
{
  int ret;
  DBT key, data;

  memset(&key, 0, sizeof(DBT));  /* Initialize the DBTs */
  memset(&data, 0, sizeof(DBT));

  key.data = hash_schema;
  key.size = strlen(hash_schema);

  data.data = buffer;
  data.ulen = MAX_BUFFER_SIZE; 
  data.flags = DB_DBT_USERMEM;
 
  ret =  db_ptr->get(db_ptr, NULL, &key, &data, 0);  /* Retrieving data from the database */

  if (ret == DB_NOTFOUND) { 
    db_ptr->err(db_ptr, ret, "The key:- %s: doesn't exist in database\n", hash_schema);
    return ret;
  }
  else {
    return 0;
  }
}

int get_max_port(DB *db_ptr, int *max_port_in, int *max_port_out)
{ 
  int ret;
  int size;
  DBC *cursorp;
  DBT key, data;
  Address *addr;
  char *buffer;
  *max_port_in = 5556;
  *max_port_out = 8100;

  buffer = malloc(MAX_BUFFER_SIZE);
  db_ptr->cursor(db_ptr, NULL, &cursorp, 0);  /* Initialize cursor */

  memset(&key, 0, sizeof(DBT));   /* Initialize our DBTs. */
  memset(&data, 0, sizeof(DBT));
 
  data.data = buffer;
  data.ulen = MAX_BUFFER_SIZE; 
  data.flags = DB_DBT_USERMEM;

  while (!(ret=cursorp->c_get(cursorp, &key, &data, DB_NEXT)))  /* Iterate over the database, retrieving each record in turn. */
    {
      //printf("%s - %s\n",(char *) key.data, (char *) data.data);

      addr = make_address_object();
      size = deserialize_address(buffer, addr);
      if(size != 0) {
	if(addr->port_in >= *max_port_in) {
	  *max_port_in = addr->port_in + 1;
	}

	if(addr->port_out >= *max_port_out) {
	  *max_port_out = addr->port_out + 1;
	}
      }
    }

  if (ret != DB_NOTFOUND) {
    fprintf(stderr,"Nothing found in the database.\n");
    return -1;
  }
 
  /* Close cursor before exit */
  if (cursorp != NULL) {
    cursorp->c_close(cursorp);
  }
 
  return 0;
}


DB *remove_db(DB *db_ptr, char *hash_schema)
{
  int ret;
  DBT key;
 
  memset(&key, 0, sizeof(DBT));  /* Initialize the DBTs */
  key.data = hash_schema;
  key.size = strlen(hash_schema);

  ret = db_ptr->del(db_ptr, NULL, &key, 0);  
  if(ret != 0) {
    if (ret == DB_NOTFOUND) {
      db_ptr->err(db_ptr, ret, "The key:- %s: could not be removed", hash_schema);
    }
  }
  else {
    printf("The key:- %s is removed from the database successfully\n", hash_schema);
    //DB *db_temp = db_ptr;
    close_bdb(db_ptr);
    db_ptr = init_bdb(db_ptr);
  }

  return db_ptr;
}

int close_bdb(DB *db_ptr)
{
  /* If the database is not NULL, close it. */
  if (db_ptr != NULL) {
    db_ptr->close(db_ptr, 0);
    return 0;
  }

  return -1; 
}

#endif
/* End of database.h */
