
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

static xmlChar *xmldoc2string(xmlDoc *doc, int *size);

xmlDoc *init_xml_doc(const char *file)
{
  xmlDoc *doc = NULL;

  xmlKeepBlanksDefault(0);
  doc = xmlReadFile(file, NULL, 0);

  if (doc == NULL) {
    printf("could not parse file %s", file);
  }

  return doc;
}

xmlChar *xmldoc2string(xmlDoc *doc, int *size)
{
  xmlChar *xmlbuff;

  xmlDocDumpFormatMemory(doc, &xmlbuff, size, 0);
  
  return xmlbuff;

}

char *xmlfile2hash(const char *file) 
{
  int xml_size;
  char *xml_char;
  char *xml_hash;
  xmlDoc *xml_doc;

 /* Creating MD5 hash of the xml xml using crypt() function */
  xml_doc = init_xml_doc((char *)file); 
  if(xml_doc == NULL) {
    // printf("Could not create XML doc");
    return NULL;
  }
  xml_char = (char *)xmldoc2string(xml_doc, &xml_size);
  xml_hash = crypt(xml_char, "$1$"); /* $1$ is MD5 */

  /* Freeing up unused memory */
  free(xml_char);
  xmlFreeDoc(xml_doc);

  return xml_hash;
}

/* End of xmlutils.c */
