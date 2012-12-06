all: helloworld.o
	gcc -o helloworld helloworld.o -lpackedobjectsd 

helloworld.o: helloworld.c
	gcc -c helloworld.c  -lpackedobjectsd -I/usr/include/libxml2 

clean:
	rm -rf *.o
	rm helloworld
