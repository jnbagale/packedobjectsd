/* A packedobjectsd test program which implements  */
/* Subscriber which receives broadcast messages */

#include <stdio.h>
#include <stdlib.h>
#include <packedobjectsd/packedobjectsd.h>

int main (int argc, char *argv [])
{
  ///////// Declarations /////////////
  packedobjectsdObject *pod_obj = NULL;
  const char *schema_file = "helloworld.xsd";

  xmlDocPtr doc_received = NULL;
  int loop = 100;
  
  //////// Initialise packedobjectsd with schema file and specify node type  //////////
  if((pod_obj = init_packedobjectsd(schema_file, SUBSCRIBER, 0)) == NULL) {
    printf("failed to init packedobjectsd");
    exit(EXIT_FAILURE);
  } 

  ///////// RECEIVING SIMPLE XML OVER SIMPLE PUB SUB CONNECTION ///////
 
  printf("Receiving broadcast messages on a sub socket\n"); 

  while(loop) {
    if((doc_received = packedobjectsd_receive(pod_obj)) == NULL){
      printf("%s", pod_strerror(pod_obj->error_code));
    }
    else {
      printf("message received successfully\n");
      xml_dump_doc(doc_received);
    }
    loop--;
  }

  //////// freeing memory ///////////
  free_packedobjectsd(pod_obj);
  xmlFreeDoc(doc_received);

  return EXIT_SUCCESS;
}
