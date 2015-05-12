/* A packedobjectsd test program which implements */
/* Publisher which sends broadcast messages */

#include <stdio.h>
#include <stdlib.h>
#include <packedobjectsd/packedobjectsd.h>

int main (int argc, char *argv [])
{
  ///////// Declarations /////////////
  packedobjectsdObject *pod_obj = NULL;
  const char *xml_file = "helloworld.xml";
  const char *schema_file = "helloworld.xsd";

  xmlDocPtr doc_sent = NULL;
  int loop = 1;
  
  //////// Initialise packedobjectsd with schema file and specify node type  //////////
  if((pod_obj = init_packedobjectsd(schema_file, PUBLISHER, 0)) == NULL) {
    printf("failed to init packedobjectsd");
    exit(EXIT_FAILURE);
  } 

  ///////// SENDING SIMPLE XML OVER SIMPLE PUB SUB CONNECTION ///////
  if((doc_sent = xml_new_doc(xml_file)) == NULL) {
    printf("failed to init xml document");
    exit(EXIT_FAILURE);
  }
  while(1) {
  /* send a normal pub message */
  printf("Broadcasting message on a pub socket\n"); 
  if(packedobjectsd_send(pod_obj, doc_sent) == -1){
    printf("%s", pod_strerror(pod_obj->error_code));
  }
  else {
    printf("message sent successfully\n");
    xml_dump_doc(doc_sent);
  }
  sleep(5);
  }
  
  //////// freeing memory ///////////
  free_packedobjectsd(pod_obj);
  xmlFreeDoc(doc_sent);

  return EXIT_SUCCESS;
}
