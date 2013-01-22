#include <stdio.h>
#include <packedobjects/packedobjects.h>

#define XML_DATA "helloworld.xml"
#define XML_SCHEMA "helloworld.xsd"

int main()
{
  packedobjectsContext *pc = NULL;
  xmlDocPtr doc = NULL;
  char *pdu = NULL;

  ///////////////////// Initialising ///////////////////
  
  // initialise packedobjects
  if ((pc = init_packedobjects(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjects");
    exit(1);
  }

  ////////////////////// Encoding //////////////////////
  
  // create an XML DOM
  if ((doc = packedobjects_new_doc(XML_DATA)) == NULL) {
    printf("did not find .xml file");
    exit(1);
  }
  // encode the XML DOM
  pdu = packedobjects_encode(pc, doc);
  if (pc->bytes == -1) {
    printf("Failed to encode with error %d.\n", pc->encode_error);
    exit(1);
  }
  // free the DOM
  xmlFreeDoc(doc);

  ////////////////////// Decoding //////////////////////
  
  // decode the PDU into DOM
  doc = packedobjects_decode(pc, pdu);
  if (pc->decode_error) {
    printf("Failed to decode with error %d.\n", pc->decode_error);
    exit(1);
  }
  // output the DOM for checking
  packedobjects_dump_doc(doc);
  // free the DOM
  xmlFreeDoc(doc);

  ////////////////////// Freeing //////////////////////

  // free memory created by packedobjects
  free_packedobjects(pc);
  
  return 0;
}
