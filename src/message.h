
#ifndef MESSAGE_H_
#define MESSAGE_H_

int send_message(void *socket, char *message, int message_length); 
char *receive_message(void *socket, int *message_length ); 

#endif
/* End of message.h */
