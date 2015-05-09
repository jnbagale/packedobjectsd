
#include <stdio.h>
#include <string.h>     /* for strlen() */
#include <stdlib.h>    /* for exit()   */
#include <unistd.h>   /* for sleep()  */
#include <inttypes.h>/* for uint64_t */
#include <time.h>   /* for cpu time */
#include <zmq.h>   /* for ZeroMQ functions */
#include <arpa/inet.h> /* for ntohl() */
#include <sys/time.h>
#include <sys/resource.h>

#include "config.h"
#include "broker.h"
#include "xmlutils.h"
#include "packedobjectsd.h"

#ifdef DEBUG_MODE

#define dbg(fmtstr, args...)						\
  (printf("libpackedobjectsd" ":%s: " fmtstr "\n", __func__, ##args))
#else
#define dbg(dummy...)
#endif

#ifdef QUIET_MODE

#define alert(dummy...)
#else
#define alert(fmtstr, args...)						\
  (fprintf(stderr, "libpackedobjectsd" ":%s: " fmtstr "\n", __func__, ##args))
#endif

static int packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash, void *filter, size_t size);
static int packedobjectsd_publish(packedobjectsdObject *pod_obj, char *schema_hash);

// COMPRESSION and HEARTBEAT will be on by default unless NO_COMPRESSION and NO_HEARTBEAT flags have been set
packedobjectsdObject *init_packedobjectsd(const char *schema_file, int node_type, int options)
{
  int ret;
  unsigned long searcher_filter;
  packedobjectsdObject *pod_obj;
 
  if ((pod_obj = (packedobjectsdObject *) malloc(sizeof(packedobjectsdObject))) == NULL) {
    alert("failed to malloc packedobjectsdObject.");
    return NULL;
  }

  /* Initialise default values */
  pod_obj->bytes_sent = -1;
  pod_obj->bytes_received = -1;
  pod_obj->sub_topic = NULL;
  pod_obj->pub_topic = NULL;
  pod_obj->node_type = node_type;
  pod_obj->error_code = UNDEFINED;
  pod_obj->server_port = DEFAULT_SERVER_PORT ; 
  pod_obj->server_address = DEFAULT_SERVER_ADDRESS;
  pod_obj->pc = init_packedobjects(schema_file, 0, 0); 

  if(pod_obj->pc == NULL) {
    alert("Failed to initialise libpackedobjects.");
    //pod_obj->error_code = INIT_PO_FAILED;
    return NULL;
  }

  // Handling the extra flags
  pod_obj->init_options = options; // to be used later 

  //  valid options can be 0, 1, 2 or 3 
  // if options = 0, both flags are not set
  // if options = 1, NO_COMPRESSION flag is set NO_HEARTBEAT flag is not set
  // if options = 2, NO_COMPRESSION flag is not set NO_HEARTBEAT flag is set
  // if options = 3, both flags are set

  if ((options & NO_HEARTBEAT) == 0) { // NO_HEARTBEAT flag is not set
    dbg("Heartbeat enabled");
  }
  else {
    dbg("Heartbeat not enabled");
  }
 
  if ((options & NO_COMPRESSION) == 0) { // NO_COMPRESSION flag is not set
    dbg("Compression enabled");
  } 
  else {
    dbg("Compression not enabled");
  }


  /* create MD5 Hash of the XML schmea */
  if((pod_obj->schema_hash = xml_to_md5hash(schema_file)) == NULL) {
    alert("Failed to create hash of the schema file.");
    // pod_obj->error_code =  INVALID_SCHEMA_FILE;
    return NULL;
  }
  dbg("schema_hash: %s\n", pod_obj->schema_hash);

  // create a temp string of schema hash as user id
  // Not used anymore. update pod schema and encode/decode request function to remove it  
  sprintf(pod_obj->uid_str, "%s", pod_obj->schema_hash);
  
  /* get broker details from the Look up server using schema hash */
  
  if((ret = getBrokerInfo(pod_obj)) == -1) {
    alert("Failed to get broker detail from server");
    return NULL;
  } // sets pod_obj->unique_id, pod_obj->publisher_endpoint and pod_obj->subscriber_endpoint

  /* create custom subscribe filter for searchers using their own unique id */
  // sprintf(resp_filter, "%lu", pod_obj->unique_id);
  searcher_filter = htonl(pod_obj->unique_id);

  switch (pod_obj->node_type) {
  case SUBSCRIBER:
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "", 0);
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      // pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }
    break;

  case PUBLISHER:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      // pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    break;
   
  case PUBSUB:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }

    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "", 0);
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    break;

  case SEARCHER:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }

    // to receive message sent by responder 
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, (void *)&searcher_filter, sizeof(searcher_filter)); 
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    dbg("Searcher's subscription filter:- %lu\n", searcher_filter);
    break;
    
  case RESPONDER:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "s", 1);  /* to receive message sent by searcher */
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    dbg("Responder's subscription filter:- s\n");
    break;
    
  case SEARES:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, (void *) &searcher_filter, sizeof(searcher_filter));
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    } 
    dbg("Searcher's subscription filter:- %lu", searcher_filter);

    ret = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, "s", 1);
    if (ret == -1){
      alert("Failed to set subscribe filter for searcher: %s", zmq_strerror (errno));
      return NULL;
    } 
    dbg("Responder's subscription filter:- s\n");

    break;
    

  default:
    alert("Invalid node type."); 
    // pod_obj->error_code = INVALID_NODE_TYPE;
    return NULL;
  }
   
  // give some time to broker for initialization 
  // so that it will not miss the first message from publishers
  sleep(1);
  return pod_obj;
}

static int packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash, void *filter, size_t size)
{
  int rc;
   
  if(pod_obj->subscriber_endpoint == NULL) {
    alert("Broker address received is NULL.");
    return -1;
  }
  else {
    dbg("Broker address receieved for subscriber: %s", pod_obj->subscriber_endpoint);
  }

  /* Prepare the zeromq subscriber context and socket */
  if ((pod_obj->subscriber_context = zmq_ctx_new()) == NULL){
    alert("Failed to initialise zeromq context for subscriber: %s", zmq_strerror (errno));
    return -1;
  }

  if((pod_obj->subscriber_socket = zmq_socket (pod_obj->subscriber_context, ZMQ_SUB)) == NULL) {
    alert("Failed to create zeromq socket for subscriber: %s", zmq_strerror (errno));
    return -1;
  }

  if((rc = zmq_connect (pod_obj->subscriber_socket, pod_obj->subscriber_endpoint)) == -1){
    alert("Failed to create zeromq socket for subscriber: %s", zmq_strerror (errno));
    return -1;
  }
  
  /* Subscribe to group by filtering the received data*/
  rc =  zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, filter, size);

  if (rc == -1){
    alert("Failed to set subscribe filter for subscriber: %s", zmq_strerror (errno));
    return -1;
  }
  else {
    dbg("Subscriber is ready to receive data from broker %s", pod_obj->subscriber_endpoint);
  }

  // free(pod_obj->subscriber_endpoint); /* Free up subscriber endpoint pointer */
  return 0;
}

static int packedobjectsd_publish(packedobjectsdObject *pod_obj, char *schema_hash)
{
  int rc; 
  
  if(pod_obj->publisher_endpoint == NULL) {
    alert("Broker address received is NULL");
    return -1;
  }
  else {
    dbg("Broker address receieved for publisher: %s", pod_obj->publisher_endpoint);
  }

  /* Prepare the context and podlisher socket */
  if (( pod_obj->publisher_context = zmq_init(1)) == NULL){
    alert("Failed to initialise zeromq context for publisher: %s", zmq_strerror (errno));
    return -1;
  }

  if((pod_obj->publisher_socket = zmq_socket (pod_obj->publisher_context, ZMQ_PUB)) == NULL){ 
    alert("Failed to create zeromq socket for publisher: %s", zmq_strerror (errno));
    return -1;
  }

  if((rc = zmq_connect (pod_obj->publisher_socket, pod_obj->publisher_endpoint)) == -1){
    alert("Failed to connect publisher: %s\n", zmq_strerror (errno));
    return -1;
  }

  dbg("Publisher is ready to send data to broker at %s",pod_obj->publisher_endpoint);

  // free(pod_obj->publisher_endpoint); /* Free up publisher endpoint pointer */
  return 0;
}

xmlDocPtr packedobjectsd_receive(packedobjectsdObject *pod_obj)
{
  /* Reading the received message */
  int i;
  int rc;
  int size;
  int64_t more;
  size_t more_size = sizeof(more);
  char *pdu = NULL;
  char *status_pdu = NULL;
  xmlDocPtr doc = NULL;
  clock_t start_time, end_time;

  if(pod_obj->subscriber_socket == NULL) {
    alert("packedobjectsd isn't initialised to receive message");
    return NULL;
  }

  if((status_pdu = receiveMessagePDU(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }

  dbg("compression status:- %s", status_pdu);
  if((rc = zmq_getsockopt(pod_obj->subscriber_socket, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  if(more) {
    if((pdu = receiveMessagePDU(pod_obj->subscriber_socket, &size)) == NULL) {
      pod_obj->error_code = RECEIVE_FAILED;
      return NULL;
    }
  }
  pod_obj->bytes_received = size;

  start_time = clock();

  if(strcmp(status_pdu,"c") == 0) {
    dbg("next message is compressed");

    doc = packedobjects_decode(pod_obj->pc, pdu);
  
    
    if (pod_obj->pc->decode_error) {
      alert("Failed to decode with error %d.", pod_obj->pc->decode_error);
      pod_obj->error_code = DECODE_FAILED;
      return NULL;
    }
  }

  else {
    dbg("Received XMl w/o compression");

   
    doc = xmlReadDoc((xmlChar *) pdu, NULL, NULL, 0);
  }

  end_time = clock();
  if(start_time != -1 && end_time != -1) {
    pod_obj->decode_cpu_time = ((float) end_time / CLOCKS_PER_SEC - (float) start_time / CLOCKS_PER_SEC) * 1000.0;
  }
  else {
    pod_obj->decode_cpu_time = -1;
  }
  //alert("CPU time average for thousand decode %g ms", pod_obj->decode_cpu_time );

  dbg("data received and decoded\n");
  free(pdu);
  return doc;
}

xmlDocPtr packedobjectsd_receive_search(packedobjectsdObject *pod_obj)
{
  int rc;
  int size;
  int64_t more;
  size_t more_size = sizeof(more);
  char *pdu;

  if(pod_obj->subscriber_socket == NULL || !(pod_obj->node_type == RESPONDER || pod_obj->node_type == SEARES)) {
    alert("packedobjectsd isn't initialised to receive search message");
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
 
  if((pdu = receiveMessagePDU(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
    
  dbg("topic:- %s", pdu);
  if((rc = zmq_getsockopt(pod_obj->subscriber_socket, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  if(more && (strcmp(pdu,"s") == 0)) {
    if((pod_obj->last_searcher_id = receiveMessagePDU(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
    }
  
    // convert received node id to host order bytes
    pod_obj->last_searcher = ntohl(*(unsigned long *)pod_obj->last_searcher_id);
    dbg("The last searcher_id is %lu", pod_obj->last_searcher);
  }
  else {
    dbg("Could not receive the searcher id");
    return NULL;
  }

  if(more) {
    xmlDocPtr doc = NULL;
    doc = packedobjectsd_receive(pod_obj);
    //dbg("Received message with topic search");
    return doc;
  }
  else {
    dbg("Could not receive with topic search");
    return NULL;
  }
}

xmlDocPtr packedobjectsd_receive_response(packedobjectsdObject *pod_obj)
{
  int rc;
  int size;
  int64_t more;
  size_t more_size = sizeof(more);
  char *pdu;
  
  if(pod_obj->subscriber_socket == NULL || !(pod_obj->node_type == SEARCHER || pod_obj->node_type == SEARES)) {
    alert("packedobjectsd isn't initialised to receive response message");
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
 
  if((pdu = receiveMessagePDU(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }

  // convert received node id to unsigned long bytes
  unsigned long topic = *(unsigned long *)pdu;    
  dbg("topic:- %lu", topic);

  if((rc = zmq_getsockopt(pod_obj->subscriber_socket, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  if(more && !(strcmp(pdu,"s") == 0)) {
    xmlDocPtr doc = NULL;
    doc = packedobjectsd_receive(pod_obj);
    //dbg("Received message with topic response");
    return doc;
  }
  else {
    dbg("Could not receive with topic response");
    return NULL;
  }
}

int send_network_byte(unsigned long network_byte, packedobjectsdObject *pod_obj)
{
  // To send network id as long instead of string by converting it to network order byte
  int rc;
  zmq_msg_t z_message;

  if((rc = zmq_msg_init_size (&z_message, sizeof(network_byte))) == -1){
    alert("Error occurred during zmq_msg_init_size(): %s", zmq_strerror (errno));
    return -1;
  }

  // Mem copy network byte to zeromq message variable
  memcpy (zmq_msg_data (&z_message), &network_byte, sizeof(network_byte));
  //dbg("mem copied %d bytes %s", zmq_msg_size(&z_message), (char *)zmq_msg_data (&z_message));

  // send back node id to the client
  if((rc = zmq_msg_send (&z_message, pod_obj->publisher_socket, ZMQ_SNDMORE)) == -1){
    alert("Error occurred during zmq_send(): %s", zmq_strerror (errno));
    return -1;
  }
  //dbg("sent network order byte %lu", network_byte);
  zmq_msg_close (&z_message);

  return 1;
}

int packedobjectsd_send(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int i;
  int rc;
  int size;
  char *pdu = NULL;
  char compression_status[2];
  clock_t start_time, end_time;

  if(pod_obj->publisher_socket == NULL) {
    alert("packedobjectsd isn't initialised to send message");
    pod_obj->error_code = SEND_FAILED;
    return -1;
  }
  start_time = clock();

  // for(i = 0; i < 3000; i++) {
  if ((pod_obj->init_options  & NO_COMPRESSION) == 0) { // COMPRESSION ENABLED
    pdu = packedobjects_encode(pod_obj->pc, doc);
    size =  pod_obj->pc->bytes;

    if (size == -1) {
      //fprintf(stderr, "Failed to encode with error %d.\n", pod_obj->pc->encode_error);
      pod_obj->error_code = ENCODE_FAILED;
      return size;
    }
    compression_status[0] = 'c';
  }
  else {
    dbg("Sending XML w/o compression");
    xmlChar *xmlbuff;

    // convert xml to string
    xmlDocDumpFormatMemory(doc, &xmlbuff, &size, 0);

    // cast xmlchar to string
    pdu = (char *) xmlbuff;
    compression_status[0] = 'p';
  }
  //  }

  end_time = clock();
  if(start_time != -1 && end_time != -1) {
    pod_obj->encode_cpu_time = ((float) end_time / CLOCKS_PER_SEC - (float) start_time / CLOCKS_PER_SEC)  * 1000.0;
  }
  else {
    pod_obj->encode_cpu_time = -1;
  }
  
  //alert("CPU time average for thousand encode %g ms", pod_obj->encode_cpu_time);


  // send "c" to notify compression enabled on next message
  // send "p" to notify compression not enabled on next message
  if((rc = sendMessagePDU(pod_obj->publisher_socket, compression_status, 1, ZMQ_SNDMORE)) == -1) {
    alert("Error occurred while sending the compression status: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }

  if((rc = sendMessagePDU(pod_obj->publisher_socket, pdu, size, 0)) == -1) {
    alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }
  pod_obj->bytes_sent = size;
  
  dbg("data encoded and sent\n");
 
  return rc;
}

int packedobjectsd_send_search(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;
 
  if(pod_obj->publisher_socket == NULL || !(pod_obj->node_type == SEARCHER || pod_obj->node_type == SEARES)) {
    alert("packedobjectsd isn't initialised to send search message");
    pod_obj->error_code = SEND_FAILED;
    return -1;
  }

  // sending "s" to notify as a search message
  if((rc = sendMessagePDU(pod_obj->publisher_socket, "s", 1, ZMQ_SNDMORE)) == -1) {
    alert("Error occurred while sending the topic search: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }
  dbg("topic sent:- s [search]");
   
  // sending unique id as network order byte
  unsigned long network_byte = htonl(pod_obj->unique_id);

  // send_network_byte adds ZMQSNDMORE flag to allow next message 
  if((rc = send_network_byte(network_byte, pod_obj)) == -1) {
    alert("Error occurred while sending the unique id: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }

  dbg("searcher id:- %lu sent as network byte %lu", pod_obj->unique_id, network_byte);

  if((rc = packedobjectsd_send(pod_obj, doc)) == -1) {
    return rc;
  } 

  pod_obj->bytes_sent++; // To include the 1 bytes used for sending the compression status 'c' or 'p' before the actual data

  return 0;
}

int packedobjectsd_send_response(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;

  if(pod_obj->publisher_socket == NULL || !(pod_obj->node_type == RESPONDER || pod_obj->node_type == SEARES)) {
    alert("packedobjectsd isn't initialised to send response message");
    pod_obj->error_code = SEND_FAILED;
    return -1;
  }

  /* create responder topic using searcher id */
  if(!pod_obj->last_searcher) {
    alert("last searcher id is not known. response may not be sent properly");
    //return -1;
  }

  // sending 'unique id' topic as network order byte
  unsigned long network_byte = htonl(pod_obj->last_searcher);

  if((rc = send_network_byte(network_byte, pod_obj)) == -1) {
    alert("Error occurred while sending the unique id: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }


  dbg("topic:- %lu", network_byte);
  dbg("size of the topic sent %d", sizeof(network_byte));

  if((rc = packedobjectsd_send(pod_obj, doc)) == -1) {
    return rc;
  } 

  pod_obj->bytes_sent++; // To include the 1 bytes used for sending the the compression status 'c' or 'p'before the actual data

  return 0;
}

void free_packedobjectsd(packedobjectsdObject *pod_obj)
{
  if(pod_obj != NULL) {
    if(pod_obj->node_type == 'S' || pod_obj->node_type == 'B' ) {
      zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_UNSUBSCRIBE, "", 0);
      zmq_close (pod_obj->subscriber_socket);
      zmq_term (pod_obj->subscriber_context);
    }

    if(pod_obj->node_type == 'P' || pod_obj->node_type == 'B') {
      zmq_close (pod_obj->publisher_socket);
      zmq_term (pod_obj->publisher_context);
    }

    free_packedobjects(pod_obj->pc);
    free(pod_obj->schema_hash);
    free(pod_obj);
    dbg("successfully freed packedobjectsd");
  }
}

const char *pod_strerror(int error_code)
{
  switch(error_code)
    {
    case INVALID_NODE_TYPE:
      return "invalid node type";
    case INVALID_SCHEMA_FILE:
      return "invalid schema file";
    case SUBSCRIBE_FAILED:
      return "failed to subscribe to broker";
    case PUBLISH_FAILED:
      return "failed to publish to broker";
    case MALLOC_FAILED:
      return "failed to allocate memory";
    case INIT_PO_FAILED:
      return "failed to initialise libpackedobjects";
    case RECEIVE_FAILED:
      return "failed to receive data from broker";
    case DECODE_FAILED:
      return "failed to decode received data";
    case SEND_FAILED:
      return "failed to send data to broker";
    case ENCODE_FAILED:
      return "failed to encode data to be sent";
    default:
      return "unknown error";
    }
}


/* End of packedobjectsd.c */
