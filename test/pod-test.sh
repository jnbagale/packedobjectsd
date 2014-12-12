#!/bin/bash
echo -e "POD test\n\n"
init()
{ 
    schemafile=../examples/helloworld.xsd
    xmlfile=../examples/helloworld.xml
    cd ../ && ./configure && make && make check && sudo make install
    echo -e "\n\nRunning packedobjectsdtest program...\n\n"
    send-receive()
    {
	for i in {1..1}
	do
	    cd src && ./packedobjectsdtest --xml $xmlfile --schema $schemafile
	done
    }
 
    send-receive
}
init

