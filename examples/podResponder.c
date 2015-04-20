/* A packedobjectsd test program which implements  */
/* Responder which receives broadcasted search messages and sends back response messages */

#include <stdio.h>
#include <stdlib.h>
#include <packedobjectsd/packedobjectsd.h>

int main (int argc, char *argv [])
{
  ///////// Declarations /////////////
  int ret;
  packedobjectsdObject *pod_obj = NULL;
  const char *schema_file = "helloworld.xsd";
  const char *xml_file = "helloworld.xml";

  xmlDocPtr doc_response = NULL;
  xmlDocPtr doc_search_received = NULL;
  int loop = 100;
  
  //////// Initialise packedobjectsd with schema file and specify node type  //////////
  if((pod_obj = init_packedobjectsd(schema_file, RESPONDER, 0)) == NULL) {
    printf("failed to init packedobjectsd");
    exit(EXIT_FAILURE);
  } 

  ///////// RECEIVING SIMPLE XML OVER SIMPLE RESPONDER CONNECTION ///////
 
  printf("Receiving search messages on a responder socket\n"); 

  while(loop) {
     /* receive a search message */
    if((doc_search_received = packedobjectsd_receive_search(pod_obj)) == NULL) {
      printf("%s", pod_strerror(pod_obj->error_code));
    }
    else {
      printf("search message received successfully\n");
      xml_dump_doc(doc_search_received);
    }

    /* PERFORM QUERY PROCESSING HERE! */

    /* send a response message */
    if((doc_response = xml_new_doc(xml_file)) == NULL) {
      printf("did not find .xml file");
    }

    if((ret = packedobjectsd_send_response(pod_obj, doc_response)) == -1){
      printf("%s", pod_strerror(pod_obj->error_code));
    }
    else {
      printf("response message sent successfully\n");
    }

    usleep(1000); // sleep for 1ms
    loop--;
  }

  //////// freeing memory ///////////
  free_packedobjectsd(pod_obj);
  xmlFreeDoc(doc_search_received);
  xmlFreeDoc(doc_response);

  return EXIT_SUCCESS;
}
