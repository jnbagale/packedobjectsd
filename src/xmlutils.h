
#ifndef XMLUTILS_H_
#define XMLUTILS_H_

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>

xmlDocPtr xml_new_doc(const char *file);
int xml_doc_size(xmlDocPtr doc);
void xml_dump_doc(xmlDocPtr doc);
char *xml_to_md5hash(const char *file); 

#endif
/* End of xmlutils.h */
