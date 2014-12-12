

#include <string.h>

#include "md5_hash.h"
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

  xmlDocDumpFormatMemory(doc, &xmlbuff, &size, 1); // 1 will include whitespaces if xmlKeepBlanksDefault was set
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
  int i;
  int xml_size;
  char *xml_char;
  char *xml_hash;
  xmlDocPtr xml_doc;

  MD5_CTX context;
  unsigned char digest[16];
  char mdString[33];
  unsigned int len;

 /* Creating MD5 hash of the xml xml using crypt() function */
  xml_doc = xml_new_doc((char *)file); 
  if(xml_doc == NULL) {
    // printf("Could not create XML doc");
    return NULL;
  }
  xml_char = (char *)xml_to_string(xml_doc, &xml_size);
  len = strlen(xml_char);

  // generate MD5 hash of the xml string

  MD5Init (&context);
  MD5Update (&context, xml_char, len);
  MD5Final (digest, &context);

  // converts message digest to hecadecimal
  for(i = 0; i < 16; i++)
    {
      sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    }

  xml_hash = strdup((char *)mdString);
  //printf("md5 digest: %s %d\n", mdString, strlen(mdString));

  /* Freeing up unused memory */
  free(xml_char);
  xmlFreeDoc(xml_doc);

  return xml_hash; 
}

/* End of xmlutils.c */
