all: video_client.o video_searcher.o
	gcc -o video_client video_client.o  -lpackedobjectsd -luuid
	gcc -o video_searcher video_searcher.o  -lpackedobjectsd -luuid

video_client.o: video_client.c
	gcc -c video_client.c  -lpackedobjectsd -I/usr/include/libxml2 -luuid
video_searcher.o: video_searcher.c
	gcc -c video_searcher.c  -lpackedobjectsd -I/usr/include/libxml2 -luuid

clean:
	rm -rf *.o
	rm video_client
	rm video_searcher