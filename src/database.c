
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
#include "database.h"
#include "message.h"

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
    exit(EXIT_FAILURE);
  }
  /* else { */
  /*   printf("Database is opened and ready for use\n"); */
  /* } */

  return db_ptr;
}


DB *write_db(DB *db_ptr, char *hash_schema)
{
  int ret;
  int size;
  DBT key, data;
  char buffer[MAX_BUFFER_SIZE];
  Address *addr;
  char *address = "127.0.0.1";

  addr = make_address_object();
  addr = create_address(addr, address , 5556, 8100);

  memset(&key, 0, sizeof(DBT)); /* Initialize the DBTs */
  memset(&data, 0, sizeof(DBT));
  
  key.data = hash_schema;
  key.size = strlen(hash_schema);

  size = serialize_address(buffer, addr); /* add checking for error on serialization */
  data.data = buffer; 
  data.size = size; 

  ret = db_ptr->put(db_ptr, NULL, &key, &data, DB_NOOVERWRITE);  /* Inserting data to the database */
  if (ret == DB_KEYEXIST) {
    db_ptr->err(db_ptr, ret, "Put failed because key %s already exists", hash_schema);
  }
  else {
    printf("The key:- %s is inserted to database successfully\n", hash_schema);
  }
  
  db_ptr = close_bdb(db_ptr);
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
    int size;
    Address *addr;
    addr = make_address_object();
    size = deserialize_address(buffer, addr);
    return size;
  }
}

void read_all_db(DB *db_ptr)
{ 
  int ret;
  DBC *cursorp;
  DBT key, data;
  
  db_ptr->cursor(db_ptr, NULL, &cursorp, 0);  /* Initialize cursor */

  memset(&key, 0, sizeof(DBT));   /* Initialize our DBTs. */
  memset(&data, 0, sizeof(DBT));
 
  while (!(ret=cursorp->c_get(cursorp, &key, &data, DB_NEXT)))  /* Iterate over the database, retrieving each record in turn. */
    {
      printf("%s - %s\n",(char *) key.data, (char *) data.data);
    }
  if (ret != DB_NOTFOUND) {
    fprintf(stderr,"Nothing found in the database.\n");
  }
 
  /* Close cursor before exit */
  if (cursorp != NULL)
    cursorp->c_close(cursorp);
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
    db_ptr = close_bdb(db_ptr);
    db_ptr = init_bdb(db_ptr);
  }

  return db_ptr;
}

DB *close_bdb(DB *db_ptr)
{
  /* If the database is not NULL, close it. */
  if (db_ptr != NULL) {
    db_ptr->close(db_ptr, 0); 
  }
  return db_ptr;
}