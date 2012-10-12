
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

xmlDoc *init_xmlutils(char *file)
{
  xmlDoc *doc = NULL;

  xmlKeepBlanksDefault(0);
  doc = xmlReadFile(file, NULL, 0);
  
  if (doc == NULL) {
    printf("error: could not parse file %s\n", file);
  }

  return doc;
 
}

xmlChar *xmldoc2string(xmlDoc *doc, int *size)
{
  xmlChar *xmlbuff;

  xmlDocDumpFormatMemory(doc, &xmlbuff, size, 0);
  
  return xmlbuff;

}

xmlDoc *xmlstring2doc(char *xmlstr, int size)
{
  return xmlParseMemory(xmlstr, size);

}

char *xmlfile2hash(const char *file_schema) 
{
  int xml_size;
  char *char_schema;
  char *hash_schema;
  xmlDoc *doc_schema;

 /* Creating MD5 hash of the xml schema using crypt() function */
  doc_schema = init_xmlutils((char *)file_schema); 
  if(doc_schema == NULL) {
    printf("The XML schema: %s doesn't exist\n", file_schema);
    return NULL;
  }
  char_schema = (char *)xmldoc2string(doc_schema, &xml_size);
  hash_schema = crypt(char_schema, "$1$"); /* $1$ is MD5 */

  /* Freeing up unused memory */
  free(char_schema);
  xmlFreeDoc(doc_schema);

  return hash_schema;
}

/* End of xmlutils.c */
