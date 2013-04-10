
#include <stdio.h>
#include <string.h>     /* for strlen() */
#include <stdlib.h>    /* for exit()   */
#include <inttypes.h> /* for uint64_t */
#include <zmq.h>     /* for ZeroMQ functions */
#include <uuid/uuid.h> 

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

static int packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash, char filter[10]);
static int packedobjectsd_publish(packedobjectsdObject *pod_obj, char *schema_hash);

packedobjectsdObject *init_packedobjectsd(const char *schema_file, int node_type)
{
  int ret;
  uuid_t buf;
  char resp_filter[100];
  packedobjectsdObject *pod_obj;
 
  if ((pod_obj = (packedobjectsdObject *) malloc(sizeof(packedobjectsdObject))) == NULL) {
    alert("failed to malloc packedobjectsdObject.");
    return NULL;
  }

  /* Initialise default values for initialisation */
  pod_obj->bytes_sent = -1;
  pod_obj->bytes_received = -1;
  pod_obj->sub_topic = NULL;
  pod_obj->pub_topic = NULL;
  pod_obj->node_type = node_type;
  pod_obj->error_code = UNDEFINED;
  pod_obj->server_port = DEFAULT_SERVER_PORT ; 
  pod_obj->server_address = DEFAULT_SERVER_ADDRESS;
  pod_obj->pc = init_packedobjects(schema_file, 0); 

  if(pod_obj->pc == NULL) {
    alert("Failed to initialise libpackedobjects.");
    //pod_obj->error_code = INIT_PO_FAILED;
    return NULL;
  }

  /* generate unique id using uuid library */
  uuid_generate_random(buf);
  uuid_unparse(buf, pod_obj->unique_id);
  dbg("unique id created successfully: %s", pod_obj->unique_id);

  /* create custom subscribe filter for searchers using their own unique id */
  sprintf(resp_filter, "r##%s", pod_obj->unique_id);

  /* create MD5 Hash of the XML schmea */
  if((pod_obj->schema_hash = xml_to_md5hash(schema_file)) == NULL) {
    alert("Failed to create hash of the schema file.");
    // pod_obj->error_code =  INVALID_SCHEMA_FILE;
    return NULL;
  }
  dbg("schema_hash: %s", pod_obj->schema_hash);

  /* get broker details from the Look up server using schema hash */
  if((ret = get_broker_detail(pod_obj)) == -1) {
    alert("Failed to get broker detail from server");
    return NULL;
  }

  switch (pod_obj->node_type) {
  case SUBSCRIBER:
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "");
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
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "");
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
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, resp_filter); /* to receive message sent by responder */
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    break;
    
  case RESPONDER:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, "s");  /* to receive message sent by searcher */
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    }  
    break;
  
  case SEARES:
    ret = packedobjectsd_publish(pod_obj, pod_obj->schema_hash);
    if(ret == -1) {
      alert("Failed to publish to packedobjectsd");
      //  pod_obj->error_code = PUBLISH_FAILED;
      return NULL;
    }
    ret = packedobjectsd_subscribe(pod_obj, pod_obj->schema_hash, resp_filter);
    if(ret == -1) {
      alert("Failed to subscribe to packedobjectsd");
      //  pod_obj->error_code =  SUBSCRIBE_FAILED;
      return NULL;
    } 

    ret = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, "s", 1);
    if (ret == -1){
      alert("Failed to set subscribe filter for searcher: %s", zmq_strerror (errno));
      return NULL;
    } 
    dbg("Subscription filter:- s");
    break;

  default:
    alert("Invalid node type."); 
    // pod_obj->error_code = INVALID_NODE_TYPE;
    return NULL;
  }
   
  return pod_obj;
}

static int packedobjectsd_subscribe(packedobjectsdObject *pod_obj, char *schema_hash, char filter[10])
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
  rc = zmq_setsockopt (pod_obj->subscriber_socket, ZMQ_SUBSCRIBE, filter, strlen(filter));
  if (rc == -1){
    alert("Failed to set subscribe filter for subscriber: %s", zmq_strerror (errno));
    return -1;
  }
  else {
    dbg("Subscriber is ready to receive data from broker %s", pod_obj->subscriber_endpoint);
    dbg("Subscription filter:- %s", filter);
  }

  free(pod_obj->subscriber_endpoint); /* Free up subscriber endpoint pointer */
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

  free(pod_obj->publisher_endpoint); /* Free up publisher endpoint pointer */
  return 0;
}


xmlDocPtr packedobjectsd_receive(packedobjectsdObject *pod_obj)
{
  /* Reading the received message */
  int size;
  char *pdu = NULL;
  xmlDocPtr doc = NULL;

  if(pod_obj->subscriber_socket == NULL) {
    alert("packedobjectsd isn't initialised to receive message");
    return NULL;
  }

  if((pdu = receive_message(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }

  pod_obj->bytes_received = size;

  doc = packedobjects_decode(pod_obj->pc, pdu);
  if (pod_obj->pc->decode_error) {
    alert("Failed to decode with error %d.", pod_obj->pc->decode_error);
    pod_obj->error_code = DECODE_FAILED;
    return NULL;
  }
  
  dbg("data received and decoded");
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

  if(pod_obj->subscriber_socket == NULL) {
    alert("packedobjectsd isn't initialised to receive search message");
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
 
  if((pdu = receive_message(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
    
  dbg("topic:- %s", pdu);
  if((rc = zmq_getsockopt(pod_obj->subscriber_socket, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  if(more) {
    if((pod_obj->last_searcher_id = receive_message(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
    }
    dbg("searcher id:- %s", pod_obj->last_searcher_id);
  }
  else {
    dbg("Could not receive the searcher id");
    return NULL;
  }

  if(more) {
    xmlDocPtr doc = NULL;
    doc = packedobjectsd_receive(pod_obj);
    dbg("Received message with topic search");
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
  
  if(pod_obj->subscriber_socket == NULL) {
    alert("packedobjectsd isn't initialised to receive response message");
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
 
  if((pdu = receive_message(pod_obj->subscriber_socket, &size)) == NULL) {
    pod_obj->error_code = RECEIVE_FAILED;
    return NULL;
  }
    
  dbg("topic:- %s", pdu);
  if((rc = zmq_getsockopt(pod_obj->subscriber_socket, ZMQ_RCVMORE, &more, &more_size)) == -1) {
    alert("Failed to get socket option");
  }

  if(more) {
    xmlDocPtr doc = NULL;
    doc = packedobjectsd_receive(pod_obj);
    dbg("Received message with topic response");
    return doc;
  }
  else {
    dbg("Could not receive with topic response");
    return NULL;
  }
}

int packedobjectsd_send(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;
  int size;
  char *pdu = NULL;

  if(pod_obj->publisher_socket == NULL) {
    alert("packedobjectsd isn't initialised to send message");
    pod_obj->error_code = SEND_FAILED;
    return -1;
  }

  pdu = packedobjects_encode(pod_obj->pc, doc);
  size =  pod_obj->pc->bytes;

  if (size == -1) {
    //fprintf(stderr, "Failed to encode with error %d.\n", pod_obj->pc->encode_error);
    pod_obj->error_code = ENCODE_FAILED;
    return size;
  }

  if((rc = send_message(pod_obj->publisher_socket, pdu, size, 0)) == -1) {
    alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }
  pod_obj->bytes_sent = size;
  
  dbg("data encoded and sent");
 
  return rc;
}

int packedobjectsd_send_search(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;
 
  if(pod_obj->publisher_socket == NULL) {
    alert("packedobjectsd isn't initialised properly");
    return -1;
  }

  if((rc = send_message(pod_obj->publisher_socket, "s", 1, ZMQ_SNDMORE)) == -1) {
    alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }

  dbg("topic:- s [search]");

  if((rc = send_message(pod_obj->publisher_socket, pod_obj->unique_id, strlen(pod_obj->unique_id), ZMQ_SNDMORE)) == -1) {
    alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }

  dbg("searcher id:- %s", pod_obj->unique_id);

  if((rc = packedobjectsd_send(pod_obj, doc)) == -1) {
    return rc;
  } 

  pod_obj->bytes_sent++;

  return 0;
}

int packedobjectsd_send_response(packedobjectsdObject *pod_obj, xmlDocPtr doc)
{
  int rc;
  char resp_topic[100];

  if(pod_obj->publisher_socket == NULL) {
    alert("packedobjectsd isn't initialised properly");
    return -1;
  }

  /* create responder topic using searcher id */
  if(pod_obj->last_searcher_id == NULL) {
    alert("last searcher id is not known. response may not sent properly");
    //return -1;
  }
  sprintf(resp_topic, "r##%s", pod_obj->last_searcher_id);

  if((rc = send_message(pod_obj->publisher_socket, resp_topic, strlen(resp_topic), ZMQ_SNDMORE)) == -1) {
    alert("Error occurred while sending the message: %s", zmq_strerror (errno));
    pod_obj->error_code = SEND_FAILED;
    return rc;
  }

  dbg("topic:- %s", resp_topic);

  if((rc = packedobjectsd_send(pod_obj, doc)) == -1) {
    return rc;
  } 

  pod_obj->bytes_sent++;

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
