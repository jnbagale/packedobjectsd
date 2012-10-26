all: simple_server.o simple_client.o multiple_server.o multiple_client.o
	gcc -o simple_server simple_server.o  -lpackedobjectsd
	gcc -o simple_client simple_client.o  -lpackedobjectsd
	gcc -o multiple_server multiple_server.o  -lpackedobjectsd
	gcc -o multiple_client multiple_client.o  -lpackedobjectsd

simple_server.o: simple_server.c
	gcc -c simple_server.c  -lpackedobjectsd -I/usr/include/libxml2
simple_client.o: simple_client.c
	gcc -c simple_client.c  -lpackedobjectsd -I/usr/include/libxml2

multiple_server.o: multiple_server.c
	gcc -c multiple_server.c  -lpackedobjectsd -I/usr/include/libxml2
multiple_client.o: multiple_client.c
	gcc -c multiple_client.c  -lpackedobjectsd -I/usr/include/libxml2

clean:
	rm -rf *.o
	rm simple_server
	rm simple_client