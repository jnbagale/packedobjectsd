sen#!/bin/bash
echo -e "POD test\n\n"
init()
{ 
    schemafile=../schema/personnel.xsd
    xmlfile=../schema/personnel.xml
    cd ../ && ./configure && make && make check
    echo -e "\n\nRunning packedobjectsdtest program...\n\n"
    send-receive()
    {
	for i in {1..1}
	do
	    cd src && ./packedobjectsdtest --xml $xmlfile --schema $schemafile  > pod-result.log 2>&1
	done
    }
 
    send-receive
    cat pod-result.log
}
init
rm pod-result.log	

