all: simple-video-server.o simple-video-client.o 
	gcc -o simple-video-server simple-video-server.o  -lpackedobjectsd
	gcc -o simple-video-client simple-video-client.o  -lpackedobjectsd

simple-video-server.o: simple-video-server.c
	gcc -c simple-video-server.c  -lpackedobjectsd -I/usr/include/libxml2
simple-video-client.o: simple-video-client.c
	gcc -c simple-video-client.c  -lpackedobjectsd -I/usr/include/libxml2

clean:
	rm -rf *.o
	rm simple-video-server
	rm simple-video-client