
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A function to create MD5 hash of xml schema file */

#include <stdio.h>
#include <string.h>  /* for strlen()  */
#include <stdlib.h>  /* for exit()   */
#include <unistd.h>  /* for read()  */
#include <fcntl.h>   /* for open() */

#define BUF_LEN 10000

char *create_schema_hash(char *filename)
{
  int fd;
  int ret;
  char buf[BUF_LEN];
  char *schema_hash;
  if ((fd = open(filename, O_RDONLY)) == -1) {
    printf("failed to open %s!\n",filename);
    exit(EXIT_FAILURE);
  }
  ret = read(fd, buf, BUF_LEN);
  if( ret == -1) {
    printf("failed to read %s!\n",filename);
    exit(EXIT_FAILURE);
  }
  close(fd);

  schema_hash = g_compute_checksum_for_string(G_CHECKSUM_MD5, buf, ret);
  printf("MD5 hash of schema %s = %s\n \n", filename, schema_hash);
  return schema_hash;
}

