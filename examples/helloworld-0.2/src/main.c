
#include <stdio.h>
#include <unistd.h>      /* for sleep() */
#include <packedobjectsd/packedobjectsd.h>

#define XML_DATA "helloworld.xml"
#define XML_SCHEMA "helloworld.xsd"


int main ()
{
  packedobjectsdObject *pod_obj = NULL;
  xmlDocPtr doc_sent = NULL;
  xmlDocPtr doc_received = NULL;

  ///////////////////// Initialising ///////////////////

  /* Initialise packedobjectsd */

  if((pod_obj = init_packedobjectsd(XML_SCHEMA)) == NULL) {
    printf("failed to initialise libpackedobjectsd");
    exit(1);
  }
  
  sleep(1); /* Allow broker to start if it's not already running */
  
  ////////////////////// Sending XML data //////////////////////
 
  /* create an XML DOM */
  if((doc_sent = xml_new_doc(XML_DATA)) == NULL) {
    printf("did not find .xml file");
    exit(1);
  }

  /* send the XML DOM */
  if(packedobjectsd_send(pod_obj, doc_sent) == -1){
    printf("failed to send with error %s", pod_strerror(pod_obj->error_code));
    exit(1);
  }

  printf("size of the original xml: %d bytes\n", xml_doc_size(doc_sent));
  printf("size after the encoding: %d bytes\n", pod_obj->bytes_sent);
  /* free the XML DOM */
  xmlFreeDoc(doc_sent);
 
  ////////////////////// Receiving XML data //////////////////////
 
  if((doc_received = packedobjectsd_receive(pod_obj)) == NULL) {
    printf("failed to receive with error %s", pod_strerror(pod_obj->error_code));
    exit(1);
  }
  
  printf("size before the decoding: %d bytes\n", pod_obj->bytes_received);
  printf("size of the decoced xml: %d bytes \n", xml_doc_size(doc_received));

  /* output the DOM for checking */
  xml_dump_doc(doc_received);
  /* free the XML DOM */
  xmlFreeDoc(doc_received);

  ////////////////////// Freeing //////////////////////

  /* free memory created by packedobjectsd */
  free_packedobjectsd(pod_obj);

  return 0;
}
