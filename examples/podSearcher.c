/* A packedobjectsd test program which implements */
/* Searcher which sends broadcast search messages and receives back response messages */

#include <stdio.h>
#include <stdlib.h>
#include <packedobjectsd/packedobjectsd.h>

int main (int argc, char *argv [])
{
  ///////// Declarations /////////////
  int ret;
  packedobjectsdObject *pod_obj = NULL;
  const char *xml_file = "helloworld.xml";
  const char *schema_file = "helloworld.xsd";

  xmlDocPtr doc_search = NULL;
  xmlDocPtr doc_response_received = NULL;
  int loop = 1;
  
  //////// Initialise packedobjectsd with schema file and specify node type  //////////
  if((pod_obj = init_packedobjectsd(schema_file, SEARCHER, 0)) == NULL) {
    printf("failed to init packedobjectsd");
    exit(EXIT_FAILURE);
  } 

  ///////// SENDING SIMPLE XML OVER SIMPLE SEARCHER CONNECTION ///////
  printf("Sending search messages on a searcher socket\n");

  if((doc_search = xml_new_doc(xml_file)) == NULL) {
    printf("did not find .xml file");
  }
  
  /* send a search message */ 
  if((ret = packedobjectsd_send_search(pod_obj, doc_search)) == -1){
    printf("%s", pod_strerror(pod_obj->error_code));
  }
  else {
    printf("search message sent successfully\n");
  }

  /////// Wait for response messages from Respnders ////////
  printf("Waiting for response messages on a searcher socket\n");

  /* USE LOOP TO RECEIVE MESSAGES CONTINUOUSLY */

  /* receive a response message */
  if((doc_response_received = packedobjectsd_receive_response(pod_obj)) == NULL) {
    printf("%s", pod_strerror(pod_obj->error_code));
  }
  else {
    printf("response message received\n");
    xml_dump_doc(doc_response_received);
  }
  
  //////// freeing memory ///////////
  free_packedobjectsd(pod_obj);
  xmlFreeDoc(doc_search);

  return EXIT_SUCCESS;
}
