
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <unistd.h>
#define _GNU_SOURCE
#include <crypt.h>  /* for crypt() */

#include "xmlutils.h"

static xmlDoc *init_xmlutils(char *schema_file);
static xmlChar *xmldoc2string(xmlDoc *doc, int *size);

xmlDoc *init_xmlutils(char *schema_file)
{
  xmlDoc *doc = NULL;

  xmlKeepBlanksDefault(0);
  doc = xmlReadFile(schema_file, NULL, 0);
  
  return doc;
}

xmlChar *xmldoc2string(xmlDoc *doc, int *size)
{
  xmlChar *xmlbuff;

  xmlDocDumpFormatMemory(doc, &xmlbuff, size, 0);
  
  return xmlbuff;

}

char *xmlfile2hash(const char *schema_file) 
{
  int xml_size;
  char *schema_char;
  char *schema_hash;
  xmlDoc *schema_doc;

 /* Creating MD5 hash of the xml schema using crypt() function */
  schema_doc = init_xmlutils((char *)schema_file); 
  if(schema_doc == NULL) {
    // printf("The XML schema: %s doesn't exist\n", schema_file);
    return NULL;
  }
  schema_char = (char *)xmldoc2string(schema_doc, &xml_size);
  schema_hash = crypt(schema_char, "$1$"); /* $1$ is MD5 */

  /* Freeing up unused memory */
  free(schema_char);
  xmlFreeDoc(schema_doc);

  return schema_hash;
}

/* End of xmlutils.c */
