INC=-I../src/ -I/usr/include/libxml2
LIB= ../src/.libs/libpackedobjectsd.so 

all: simple_server.o simple_searcher.o simple_receiver.o multiple_server.o multiple_client.o
	gcc -o simple_server simple_server.o   $(LIB) 
	gcc -o simple_searcher simple_searcher.o  $(LIB) 
	gcc -o simple_receiver simple_receiver.o   $(LIB)
	gcc -o multiple_server multiple_server.o   $(LIB)
	gcc -o multiple_client multiple_client.o   $(LIB)

simple_server.o: simple_server.c
	gcc -c simple_server.c  $(INC) 
simple_searcher.o: simple_searcher.c
	gcc -c simple_searcher.c  $(INC)
simple_receiver.o: simple_receiver.c
	gcc -c simple_receiver.c  $(INC)
multiple_server.o: multiple_server.c
	gcc -c multiple_server.c  $(INC) 
multiple_client.o: multiple_client.c
	gcc -c multiple_client.c  $(INC)

clean:
	rm -rf *.o
	rm simple_server
	rm simple_searcher
	rm simple_receiver
	rm multiple_client
	rm multiple_server