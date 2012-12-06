
#ifndef ADDRESS_H_
#define ADDRESS_H_

#define MAX_ADDRESS_SIZE 1000 /* the maximum size for network address */

typedef struct {
  int pid;
  unsigned int port_in;
  unsigned int port_out;
  char *address;  
} Address;

Address *make_address_object(); 
Address *create_address(Address *addr, char *address, int port_in, int port_out, int pid); 
void free_address_object(Address *addr); 
int serialize_address(char *buffer, Address *addr); 
int deserialize_address(char *buffer, Address *addr);

#endif
/* End of address.h */
