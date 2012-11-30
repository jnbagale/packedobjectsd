
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <unistd.h>
#define _GNU_SOURCE
#include <crypt.h>  /* for crypt() */

#include "xmlutils.h"

static xmlChar *xml_to_string(xmlDocPtr doc, int *size);

xmlDocPtr xml_new_doc(const char *file)
{
  xmlDocPtr doc = NULL;

  xmlKeepBlanksDefault(0);
  doc = xmlReadFile(file, NULL, 0);

  if (doc == NULL) {
    printf("could not parse file %s", file);
  }

  return doc;
}

int xml_doc_size(xmlDocPtr doc)
{
  int size = -1;
  xmlChar *xmlbuff;

  xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &size,"UTF-8", 1);
  free(xmlbuff);

  return size;
}

void xml_dump_doc(xmlDocPtr doc)
{
  xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
}

xmlChar *xml_to_string(xmlDocPtr doc, int *size)
{
  xmlChar *xmlbuff; 

  xmlDocDumpFormatMemory(doc, &xmlbuff, size, 0); /* xmlbuff to be freed with xmlFree() by the caller of this function */
  
  return xmlbuff;

}

char *xml_to_md5hash(const char *file) 
{
  int xml_size;
  char *xml_char;
  char *xml_hash;
  xmlDocPtr xml_doc;

 /* Creating MD5 hash of the xml xml using crypt() function */
  xml_doc = xml_new_doc((char *)file); 
  if(xml_doc == NULL) {
    // printf("Could not create XML doc");
    return NULL;
  }
  xml_char = (char *)xml_to_string(xml_doc, &xml_size);
  xml_hash = crypt(xml_char, "$1$"); /* $1$ is MD5 */

  /* Freeing up unused memory */
  free(xml_char);
  xmlFreeDoc(xml_doc);

  return xml_hash; 
}

/* End of xmlutils.c */
