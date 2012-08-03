
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

//#include <libxml/xmlschemas.h>
//#include <libxml/xmlschemastypes.h>
#include "xmlutils.h"

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

/* End of xmlutils.c */
