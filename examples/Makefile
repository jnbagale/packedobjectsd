all: video_server.o video_searcher.o video_receiver.o 
	gcc -o video_server video_server.o   -lpackedobjectsd 
	gcc -o video_searcher video_searcher.o  -lpackedobjectsd 
	gcc -o video_receiver video_receiver.o   -lpackedobjectsd

video_server.o: video_server.c
	gcc -c video_server.c  -lpackedobjectsd -I/usr/include/libxml2 
video_searcher.o: video_searcher.c
	gcc -c video_searcher.c  -lpackedobjectsd -I/usr/include/libxml2
video_receiver.o: video_receiver.c
	gcc -c video_receiver.c  -lpackedobjectsd -I/usr/include/libxml2

clean:
	rm -rf *.o
	rm video_server
	rm video_searcher
	rm video_receiver
