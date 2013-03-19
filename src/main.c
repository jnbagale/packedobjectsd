/* A packedobjectsd test program which implements both publisher and subscriber */
/* Subscriber connects to server and receives messages */
/* Publisher connects to server and sends messages */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   /* for strcmp()*/
#include <unistd.h>  /* for sleep() */
#include <getopt.h>

#include "packedobjectsd.h"

static int verbose_flag;
static unsigned send_count = 0;
static unsigned receive_count = 0;

static void send_file(packedobjectsdObject *pod_obj, const char *xml_file);
static void receive_file(packedobjectsdObject *pod_obj);
static void exit_with_message(const char *message);
static void print_usage(void);

static void send_file(packedobjectsdObject *pod_obj, const char *xml_file)
{
  int ret;
  xmlDocPtr doc_sent = NULL;

  if((doc_sent = xml_new_doc(xml_file)) == NULL) {
    exit_with_message("did not find .xml file");
  }

  if((ret = packedobjectsd_send(pod_obj, doc_sent)) == -1){
    exit_with_message(pod_strerror(pod_obj->error_code));
  }
  send_count++;
  printf("message sent\n");
  //xml_dump_doc(doc_sent);
  xmlFreeDoc(doc_sent);
}

static void receive_file(packedobjectsdObject *pod_obj)
{ 
  xmlDocPtr doc_received = NULL;

  if((doc_received = packedobjectsd_receive(pod_obj)) == NULL) {
    exit_with_message(pod_strerror(pod_obj->error_code));
  }
  receive_count++;
  printf("message received\n");
  //xml_dump_doc(doc_received);
  xmlFreeDoc(doc_received);
  
}

static void exit_with_message(const char *message)
{
  printf("Failed to run: %s\n", message);
  exit(EXIT_FAILURE);
}

static void print_usage(void)
{
  printf("usage: packedobjectsdtest --schema <file> --xml <file> \n");
  exit(EXIT_SUCCESS);
}

int main (int argc, char *argv [])
{
  packedobjectsdObject *pod_obj = NULL;
  const char *xml_file = NULL;
  const char *schema_file = NULL;
  int loop = 1;
  int c;

  while(1) {
    static struct option long_options[] =
      {
        {"verbose", no_argument, &verbose_flag, 1},
        {"help",  no_argument, 0, 'h'},
        {"schema",  required_argument, 0, 's'},
        {"xml",  required_argument, 0, 'x'},
        {"loop",  required_argument, 0, 'l'},        
        {0, 0, 0, 0}
      };

    int option_index = 0;
    c = getopt_long (argc, argv, "hs:x:l:?", long_options, &option_index);
    if (c == -1) break;
    switch (c)
      {
      case 0:
        if (long_options[option_index].flag != 0) break;
        printf ("option %s", long_options[option_index].name);
        if (optarg) printf (" with arg %s", optarg);
        printf ("\n");
        break;
      case 'h':
        print_usage();
        break;  
      case 's':
	schema_file = optarg;
        break;
      case 'x':
        xml_file = optarg;
        break;
      case 'l':
        loop = atoi(optarg);
        break;        
      case '?':
        print_usage();
        break;
      default:
        abort ();
      }
  }

  //do some simple checking

  if (!schema_file) exit_with_message("did not specify --schema file");
  if (!xml_file) exit_with_message("did not specify --xml file");

  /* Initialise packedobjectsd */
  if((pod_obj = init_packedobjectsd(schema_file, PUBSUB)) == NULL) {
    exit_with_message("failed to init packedobjectsd");
  }
  sleep(1); /* Allow broker to start if it's not already running */

  while(loop) {
    send_file(pod_obj, xml_file);
    receive_file(pod_obj);
    usleep(1000); /* Do nothing for 1 ms */
    loop--;
  }
  printf("\nTotal messages sent = %d \nTotal messages received = %d\n", send_count, receive_count);
  /* free packedobjectsd */
  free_packedobjectsd(pod_obj);

  return EXIT_SUCCESS;
}

/* End of packedobjectsdtest.c */
