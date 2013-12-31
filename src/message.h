
#ifndef MESSAGE_H_
#define MESSAGE_H_

int sendMessagePDU(void *socket, char *message, int message_length, int more); 
char *receiveMessagePDU(void *socket, int *message_length ); 

#endif
/* End of message.h */
