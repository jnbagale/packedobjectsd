

#include <string.h>
#include <openssl/md5.h>

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
  int i;
  int xml_size;
  char *xml_char;
  char *xml_hash;
  xmlDocPtr xml_doc;
  char mdString[33];
  unsigned char digest[MD5_DIGEST_LENGTH];

 /* Creating MD5 hash of the xml xml using crypt() function */
  xml_doc = xml_new_doc((char *)file); 
  if(xml_doc == NULL) {
    // printf("Could not create XML doc");
    return NULL;
  }
  xml_char = (char *)xml_to_string(xml_doc, &xml_size);
  // xml_hash = crypt(xml_char, "$1$"); /* $1$ is MD5 */



  // generate MD5 hash of the xml string
  MD5((unsigned char*)xml_char, strlen(xml_char), (unsigned char*)&digest);

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
