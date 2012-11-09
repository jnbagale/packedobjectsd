
/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

#ifndef XMLUTILS_H_
#define XMLUTILS_H_

#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>

xmlDocPtr init_xml_doc(const char *file);
void xml_dump_doc(xmlDocPtr doc);
char *xmlfile2hash(const char *file); 

#endif
/* End of xmlutils.h */
